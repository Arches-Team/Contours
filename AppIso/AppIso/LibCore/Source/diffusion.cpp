#include "diffusion.h"
#include "gpu-shader.h"
#include "cpu.h"

/*!
\class Diffusion diffusion.h
\brief Diffusion equation.

Use example :
\code
ScalarField2 base_values = ...;
ScalarField2 mask = ...;
Diffusion diff = Diffusion(base_values, mask);
diff.BuildGL();
diff.CompleteVcycle();
ScalarField2 hf = diff.GetResult();
\endcode
*/

/*!
\brief Set up diffusion.
\param v Constraint values.
\param m Mask.
*/
Diffusion::Diffusion(const ScalarField2& v, const ScalarField2& m) : Array2(v), constraints_values(v), diffused(v), mask(m)
{
}

Diffusion::~Diffusion()
{
  glDeleteBuffers(1, &in_diffused_buffer);
  glDeleteBuffers(1, &out_diffused_buffer);
  glDeleteBuffers(1, &in_mask_buffer);
  glDeleteBuffers(1, &in_constraints_buffer);

  release_program(shader_program);
}

/*!
\brief Build OpenGL resources.
*/
void Diffusion::BuildGL()
{
  // init global size variables
  total_buffer_size = nx * ny;
  dispatch_size = (std::max(nx, ny) / 8) + 1;

  // read shader.glsl
  if (shader_program == 0)
  {
    QString fullPath = System::GetResource("ARCHESLIBDIR");
    if (fullPath.isEmpty())
    {
      std::cout << "void Diffusion::BuildGL() : variable d'environnement ARCHESLIBDIR non d�fini" << std::endl;
      std::cin.get();
      exit(-1);
    }
    fullPath += "/LibCore/LibCore/Shaders/diffusion.glsl";

    if (fullPath.isEmpty())
    {
      std::cout << "Diffusion::BuildGL() : shader file not found" << std::endl;
      std::cin.get();
      exit(-1);
    }
    QByteArray ba = fullPath.toLocal8Bit();
    shader_program = read_program(ba.data());
  }

  // allocate buffers
  if (in_diffused_buffer == 0)
    glGenBuffers(1, &in_diffused_buffer);
  if (out_diffused_buffer == 0)
    glGenBuffers(1, &out_diffused_buffer);
  if (in_mask_buffer == 0)
    glGenBuffers(1, &in_mask_buffer);
  if (in_constraints_buffer == 0)
    glGenBuffers(1, &in_constraints_buffer);
  if (in_laplacian_buffer == 0)
    glGenBuffers(1, &in_laplacian_buffer);

}

/*!
\brief Initialize OpenGL buffers.
\param lap_target Target laplacian.
\param start Starting field.
*/
void Diffusion::InitBuffersGL(const ScalarField2& lap_target, const ScalarField2& start) const
{
  int ltnx = lap_target.GetSizeX();
  int ltny = lap_target.GetSizeY();
  int current_tbs = nx * ny;

  // resize constraints and mask
  ScalarField2 current_constraints = constraints_values;
  ScalarField2 current_mask = mask;
  if (ltnx != nx || ltny != ny)
  {
    current_constraints = current_constraints.Resized(nx, ny);
    current_mask = current_mask.Resized(nx, ny);
  }
  // create int mask
  std::vector<int> mask_data(current_tbs, 0.);
  for (int i = 0; i < current_tbs; i++)
  {
    mask_data[i] = (current_mask.at(i) > 0.);
  }

  // send data to gpu
  glUseProgram(shader_program);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_diffused_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * current_tbs, &start.GetAsFloats()[0], GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_diffused_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * current_tbs, &start.GetAsFloats()[0], GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_mask_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * current_tbs, mask_data.data(), GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_constraints_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * current_tbs, &current_constraints.GetAsFloats()[0], GL_STREAM_READ);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_laplacian_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * current_tbs, &lap_target.GetAsFloats()[0], GL_STREAM_READ);

  // Uniforms - just once
  glUniform1i(glGetUniformLocation(shader_program, "buffer_size_x"), nx);
  glUniform1i(glGetUniformLocation(shader_program, "buffer_size_y"), ny);

  glUseProgram(0);
}

/*!
\brief Dispatch compute shader steps.
\param nb_steps Number of steps.
\param disp_size Dispatch size.
*/
void Diffusion::DispatchStepsGL(int nb_steps, int disp_size) {
  glUseProgram(shader_program);
  for (int i = 0; i < nb_steps; i++)
  {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, in_diffused_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, out_diffused_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, in_mask_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, in_constraints_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, in_laplacian_buffer);

    glDispatchCompute(disp_size, disp_size, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // dual buffering
    std::swap(in_diffused_buffer, out_diffused_buffer);
  }
  glUseProgram(0);
}

/*!
\brief Get data back from GPU.
\param result Result field.
*/
void Diffusion::GetDataBack(ScalarField2& result) const
{
  // temporary float array for OpenGL.
  std::vector<float> data(result.VertexSize(), 0.0);

  // get data from gpu
  glGetNamedBufferSubData(in_diffused_buffer, 0, sizeof(float) * result.VertexSize(), data.data());

  // fill ScalarField2
  for (int i = 0; i < result.VertexSize(); i++)
    result[i] = double(data[i]);
}

/*!
\brief Compute unitless laplacian of a field.
\param field Input field.
\return Laplacian field.
*/
ScalarField2 Diffusion::UnitlessLaplacian(const ScalarField2& field) const
{
  ScalarField2 lap = ScalarField2(field, 0.);

  for (int i = 0; i < field.GetSizeX(); i++)
  {
    for (int j = 0; j < field.GetSizeY(); j++)
    {
      double l = 0.0;
      QPoint p = QPoint(i, j);
      int cpt = 0;
      for (int k = 0; k < 4; k++)
      {
        QPoint q = field.Next(p, 2 * k);
        if (!field.InsideVertexIndex(q)) continue;
        cpt++;
        l += field.at(q);
      }
      l /= std::max(cpt, 1);
      l -= field.at(i, j);
      lap(p) = l;
    }
  }

  return lap;
}

/*!
\brief Perform a V-cycle step.
\param lap_target Target laplacian.
\param start Starting field.
\param nb_iter Number of iterations.
\return Result field.
*/
ScalarField2 Diffusion::VcycleStep(const ScalarField2& lap_target, const ScalarField2& start, int nb_iter)
{
  int ltnx = lap_target.GetSizeX();
  int ltny = lap_target.GetSizeY();
  int max_ltnxy = std::max(ltnx, ltny);
  int min_ltnxy = std::min(ltnx, ltny);
  int disp_size = std::max(ltnx, ltny) / 8 + 1;
  std::cout << ltnx << " " << ltny << " " << nb_iter << std::endl;

  // send data to gpu
  InitBuffersGL(lap_target, start);

  // dispatch compute
  DispatchStepsGL(nb_iter, disp_size);

  // get result back
  ScalarField2 result = lap_target;
  GetDataBack(result);

  if (min_ltnxy < 40) return result;

  // compute init data for next lower level
  ScalarField2 epsilon = lap_target - UnitlessLaplacian(result);
  epsilon = epsilon.Resized(nx / 2, ny / 2);
  ScalarField2 zeroes = ScalarField2(epsilon, 0.0);

  // send next lower level VcycleStep
  ScalarField2 epsilon_result = VcycleStep(epsilon, zeroes, iter_coeff * max_ltnxy);
  epsilon_result = epsilon_result.Resized(nx, ny);

  // restart compute with better start
  InitBuffersGL(lap_target, result + epsilon_result);
  DispatchStepsGL(nb_iter, disp_size);
  GetDataBack(result);

  return result;
}

/*!
\brief Perform a complete V-cycle.
*/
void Diffusion::CompleteVcycle()
{
  ScalarField2 zeroes = ScalarField2(diffused, 0.0);
  diffused = VcycleStep(zeroes, diffused, iter_coeff * std::max(diffused.GetSizeX(), diffused.GetSizeY()));
}

