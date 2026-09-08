#include "ms_spe_gpu.h"
#include "gpu-shader.h"

void MS_SPE_GPU::Init(const HeightField& hf) {
    // init gluint to avoid warnings
    shader_program = 0;

    in_terrain_buffer = 0;
    out_terrain_buffer = 0;

    in_stream_buffer = 0;
    out_stream_buffer = 0;

    // class attributes
    nx = hf.GetSizeX();
    ny = hf.GetSizeY();
    total_buffer_size = hf.VertexSize();
    dispatch_size = (std::max(nx, ny) / 8) + 1;

    // temporary vectors to fill gpu buffers later
    std::vector<float> stream(total_buffer_size, 0.);
    std::vector<float> terrain(total_buffer_size, 0.);
    std::vector<float> hardness(total_buffer_size, 0.5);
    for (int i = 0; i < total_buffer_size; i++) {
        //std::cout << hf.at(i) << std::endl;
        terrain[i] = hf.at(i);
    }

    // create & load shader
    if (shader_program == 0) {
        QString fullPath = System::GetResource("ARCHESLIBDIR", "/LibHeightfield/LibHeightfield/Shaders/ms_spe_gpu.glsl");

        if (fullPath.isEmpty()) {
            std::cout << "MS_SPE_GPU::MS_SPE_GPU(const HeightField& hf) : shader file not found" << std::endl;
            std::cin.get();
            exit(-1);
        }

        QByteArray ba = fullPath.toLocal8Bit();
        shader_program = read_program(ba.data());
    }

    // create buffers
    if (in_terrain_buffer == 0)
        glGenBuffers(1, &in_terrain_buffer);
    if (out_terrain_buffer == 0)
        glGenBuffers(1, &out_terrain_buffer);
    if (in_stream_buffer == 0)
        glGenBuffers(1, &in_stream_buffer);
    if (out_stream_buffer == 0)
        glGenBuffers(1, &out_stream_buffer);
    if (in_delta_h_buffer == 0)
        glGenBuffers(1, &in_delta_h_buffer);
    if (out_delta_h_buffer == 0)
        glGenBuffers(1, &out_delta_h_buffer);
    if (in_hardness_buffer == 0)
        glGenBuffers(1, &in_hardness_buffer);

    // fill buffers
    glUseProgram(shader_program);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_terrain_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &terrain.front(), GL_STREAM_READ);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_terrain_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &stream.front(), GL_STREAM_READ);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_stream_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &stream.front(), GL_STREAM_READ);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_stream_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &stream.front(), GL_STREAM_READ);
 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_delta_h_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &stream.front(), GL_STREAM_READ);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_delta_h_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &stream.front(), GL_STREAM_READ);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_hardness_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &hardness.front(), GL_STREAM_READ);

    // fill uniforms
    Box2 box = hf.Array2::GetBox();
    Vector2 cellDiag = hf.CellDiagonal();
    glUniform1i(glGetUniformLocation(shader_program, "nx"), nx);
    glUniform1i(glGetUniformLocation(shader_program, "ny"), ny);
    glUniform2f(glGetUniformLocation(shader_program, "cellDiag"), float(cellDiag[0]), float(cellDiag[1]));
    glUniform2f(glGetUniformLocation(shader_program, "a"), float(box[0][0]), float(box[0][1]));
    glUniform2f(glGetUniformLocation(shader_program, "b"), float(box[1][0]), float(box[1][1]));

    glUseProgram(0);

    processingShader.Init(nx, ny, cellDiag[0]);
    addShader.Init(nx, ny);
}

MS_SPE_GPU::~MS_SPE_GPU() {
    glDeleteBuffers(1, &in_terrain_buffer);
    glDeleteBuffers(1, &out_terrain_buffer);

    glDeleteBuffers(1, &in_stream_buffer);
    glDeleteBuffers(1, &out_stream_buffer);

    glDeleteBuffers(1, &in_hardness_buffer);

    release_program(shader_program);
}

void MS_SPE_GPU::SetK(float k) {
    glUseProgram(shader_program);
    glUniform1f(glGetUniformLocation(shader_program, "k"), k);
    glUseProgram(0);
}

void MS_SPE_GPU::SetHardness(const ScalarField2& hardness) {
    addShader.SetCoeffMap(hardness);
    /*glUseProgram(shader_program);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_hardness_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &hardness.GetAsFloats()[0], GL_STREAM_READ);
    glUseProgram(0);*/
}

void MS_SPE_GPU::Step(int nb_steps) {
    for (int i = 0; i < nb_steps; i++) {
        // compute delta from spe
        glUseProgram(shader_program);

        glUniform1i(glGetUniformLocation(shader_program, "nx"), nx);
        glUniform1i(glGetUniformLocation(shader_program, "ny"), ny);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, in_terrain_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, out_terrain_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, in_stream_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, out_stream_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, in_delta_h_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, out_delta_h_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, in_hardness_buffer);

        glDispatchCompute(dispatch_size, dispatch_size, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glUseProgram(0);
     
        // blur delta
        processingShader.Step(smooth_steps, in_delta_h_buffer, out_delta_h_buffer);

        // add blurred delta
        addShader.Step(in_terrain_buffer, in_delta_h_buffer, out_terrain_buffer);

        std::swap(in_terrain_buffer, out_terrain_buffer);
        std::swap(in_stream_buffer, out_stream_buffer);

    }

    glUseProgram(0);
}

void MS_SPE_GPU::GetResult(HeightField& hf) {
    std::vector<float> tmpData(total_buffer_size, 0.);
    glGetNamedBufferSubData(in_terrain_buffer, 0, sizeof(float) * total_buffer_size, tmpData.data());

    for (int i = 0; i < total_buffer_size; i++)
        hf[i] = double(tmpData[i]);
}

// // ------------------------------------------------- map add -------------------------------------------------

GPUMapAdd::GPUMapAdd(const QString& _shader_program_name) {
    shader_program_name = _shader_program_name;
}

GPUMapAdd::~GPUMapAdd() {
    glDeleteBuffers(1, &coeff_buffer);
    release_program(shader_program_id);
}

void GPUMapAdd::Init(int _buffer_size_x, int _buffer_size_y) {
    buffer_size_x = _buffer_size_x;
    buffer_size_y = _buffer_size_y;
    total_buffer_size = buffer_size_x * buffer_size_y;
    dispatch_size = (std::max(buffer_size_x, buffer_size_x) / 8) + 1;

    if (shader_program_id == 0) {
        QString fullPath = shader_program_name;
        QByteArray ba = fullPath.toLocal8Bit();
        shader_program_id = read_program(ba.data());
    }

    std::vector<float> coeff(total_buffer_size, 0.5);

    if (coeff_buffer == 0)
        glGenBuffers(1, &coeff_buffer);

    glUseProgram(shader_program_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, coeff_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &coeff.front(), GL_STREAM_READ);

    glUniform1i(glGetUniformLocation(shader_program_id, "buffer_size_x"), buffer_size_x);
    glUniform1i(glGetUniformLocation(shader_program_id, "buffer_size_y"), buffer_size_y);

    glUseProgram(0);
}

void GPUMapAdd::SetCoeffMap(const ScalarField2& coeff_map) {
    glUseProgram(shader_program_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, coeff_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_buffer_size, &coeff_map.GetAsFloats()[0], GL_STREAM_READ);
    glUseProgram(0);
}

void GPUMapAdd::Step(GLuint in, GLuint add, GLuint out) {
    glUseProgram(shader_program_id);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, in);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, add);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, out);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, coeff_buffer);

    glDispatchCompute(dispatch_size, dispatch_size, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(0);
}