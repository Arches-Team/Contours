#pragma once

#include "gpu-wrapper.h"
#include "camera.h"
#include "evector.h"
#include "scalarfield.h"
#include "ray.h"
#include "frame.h"

// // Utility class for profiling CPU & GPU
typedef std::chrono::time_point<std::chrono::high_resolution_clock> MyChrono;

class RenderingProfiler
{
public:
  bool enabled = false;			//!< Flag linked to UI.

  GLuint query;					//!< GL Query for stats
  GLuint64 elapsedTimeGPU;		//!< GPU rendering time for a frame.

  int nbframes = 0;				//!< CPU Frame counter.
  MyChrono start;					//!< CPU profiler.
  double msPerFrame = 0;			//!< Recorded info.
  double framePerSecond = 0;		//!< Recorded info. 
  int drawCallPerFrame = 0;		//!< Recorded info.

  /*!
  \brief Init the profiler. Only has to be done once in the program.
  */
  inline void Init()
  {
    glGenQueries(1, &query);
    start = std::chrono::high_resolution_clock::now();
  }

  /*!
  \brief Starts profiling the GPU if enabled.
  */
  inline void BeginGPU()
  {
    if (enabled)
      glBeginQuery(GL_TIME_ELAPSED, query);
  }

  /*!
  \brief Ends the GPU profiling if enabled.
  */
  inline void EndGPU()
  {
    if (enabled)
    {
      glEndQuery(GL_TIME_ELAPSED);
      int done = 0;
      while (!done)
        glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
      glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsedTimeGPU);
    }
  }

  /*!
  \brief Update the CPU profiling.
  */
  inline void Update()
  {
    nbframes++;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
    double seconds = double(microseconds) / 1000000.0;
    if (seconds >= 1.0)
    {
      msPerFrame = seconds * 1000.0 / nbframes;
      framePerSecond = nbframes / seconds;
      nbframes = 0;
      start = std::chrono::high_resolution_clock::now();
    }
  }
};

enum class MeshMaterial
{
  Normal = 0,
  Color = 1,
  Aspect = 2,
  Texture = 3,
};

enum class MeshShading
{
  Triangles = 0,
  Lines = 1,
};

class MeshWidget : public QOpenGLWidget
{
  // Must include this if you use Qt signals/slots
  Q_OBJECT

protected:
  // Internal definition of renderable mesh
  class MeshGL
  {
  public:
    bool enabled;				//!< Render flag. Mesh is not rendered if enabled equals false.
    GLuint vao;					//!< Mesh VAO.
    GLBuffer fullBuffer;		//!< Mesh buffer. Contains 3D vertices, 3D normals, and possibly colors.
    GLBuffer indexBuffer;		//!< Mesh index buffer.
    int triangleCount;			//!< Number of triangles.
    float TRSMatrix[16];		//!< Translation-Rotation-Scale matrix (computed from a FrameScaled).

    MeshShading shading;		//!< Render flag.
    MeshMaterial material;		//!< Render flag.
    bool useWireframe;			//!< Render flag.

    GLuint textureID;            //!< OpenGL texture ID
    bool hasTexture;

  public:
    MeshGL();

    void Delete();
    void SetFrame(const FrameScaled& fr);
  };

protected:
  // Scene
  int x0 = 0, y0 = 0;  //!< Reference mouse coordinates.
  Camera camera; //!< %Camera.
  bool perspectiveProjection;
  float cameraOrthoSize;
  bool MoveAt = false;
  Vector currentAt = Vector::Null;
  Vector toAt = Vector::Null;
  int stepAt = 0;
  Vector2 nearAndFarPlane = Vector2(1.0, 50000.0); //!< Near and far distance.

  // Skybox
  bool useSkyShader = true;
  Color backgroundColor;
  GLShader skyShader;
  GLuint skyboxVAO = 0;

  // Profiler & panel flags
  RenderingProfiler profiler;
  bool renderCameraPanel = false;

  // Meshes
  GLShader meshShader;
  QMap<QString, MeshGL*> objects;

  // Boxes
  GLShader boxShader;
  QMap<QString, MeshGL*> boxObjects;

public:
  MeshWidget(QWidget* parent = nullptr);
  ~MeshWidget();

  void ClearAll();

  void SetCamera(const Camera& cam);
  void SetNearAndFarPlane(const Vector2&);
  Camera GetCamera() const;
  Ray ConvertPixelToRay(const QPoint&) const;
  void SaveScreen(int = 1920, int = 1080);

protected:
  virtual void initializeGL();
  virtual void resizeGL(int, int);
  virtual void paintGL();
  virtual void RenderSky();
  virtual void RenderUiPanels();
  virtual void RenderMeshes();
  virtual void ReloadShaders();

signals:
  void _signalUpdate();
  void _signalMouseMove();
  void _signalMouseRelease();
  void _signalMouseMoveEdit(const Ray&);
  void _signalEditSceneLeft(const Ray&);
  void _signalEditSceneRight(const Ray&);

public slots:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void mouseDoubleClickEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void wheelEvent(QWheelEvent*);
  virtual void keyPressEvent(QKeyEvent*);
  virtual void keyReleaseEvent(QKeyEvent*);
};

class TerrainRaytracingWidget : public MeshWidget
{
  // Must include this if you use Qt signals/slots
  Q_OBJECT

protected:
  // CPU Data
  ScalarField2* sf = nullptr;				//!< Pointer to heightfield.
  Box2 bbox;						//!< Bounding box of the heightfield.
  float zMin = 0.0, zMax = 0.0;				//!< Min/max elevation of the heighfield.
  float K = 1.0;						//!< Global Lipschitz constant of the heightfield.
  int nx = 0, ny = 0;						//!< Grid sizes.

  // GPU Data
  GLuint shaderProgram = 0;			//!< GL program for shader.
  GLuint raytraceVAO = 0;				//!< Raytracer VAO.
  GLBuffer hfBuffer;				//!< Heightfield elevation buffer.
  GLBuffer shadingBuffer;			//!< Shading buffer.
  std::vector<float> tmpData;		//!< Temporary float vector.

  GLuint albedoTextureBuffer = 0;			//!< Albedo texture buffer.
  GLuint externAlbedo = 0;			//!< Extern albedo texture buffer.
  bool useAlbedo = false;				//!< Albedo texture flag.
  bool useExternAlbedo = false;
  bool useWireframe = false;			//!< Wireframe flag.
  bool useCost = false;				//!< Cost shading flag.
  bool useElevationShading = false;	//!< Elevation shading flag.
  bool useGreenBrownYellow = false;	//!< Shading from color map flag.
  bool useShadingBuffer = false;		//!< Additional shading buffer use flag.
  float cameraAngleOfViewV = 0.0;

  bool show_sides = true;				//!< Flag for showing sides of the terrain
  // render resolution to set in order to display corretly the renderer zone when pressing R
  int render_res_x = 2560;
  int render_res_y = 1440;

public:
  TerrainRaytracingWidget(QWidget* = nullptr);
  ~TerrainRaytracingWidget();

  virtual void SetHeightField(ScalarField2*);
  virtual void UpdateInternal();

  void SetCamera(const Camera&);

  class PBR_Render_Options {
  public:
    // image size
    int width = 1920;  // 7680
    int height = 1080;  // 4320

    // sampling ((1, 1) for fastest shoot, (4, 2) for good quality)
    int nb_dispatch = 1;
    int anti_aliasing = 1;
    int nb_samples_per_shader = 128;

    // lights  (put to 0.75 to get neutral white light)
    Color light_1 = Color(184, 209, 255);
	Color light_2 = Color(255, 235, 186);

    // isolines (default values mean no isolines, change only one the 3 following to display them)
    std::vector<float> iso_heights = {};
    float iso_wave_length = -1.f;
    int nb_iso = -1;
    Color main_isolines_color = Color::Black;
    Color lesser_isolines_color = Color(0.4);
    float main_isolines_width = 30.f;
    float lesser_isolines_width = 15.f;
    int main_isolines_period = 4;
    bool orthogonal_projection = false;
    // sea
    Color sea_shore_color = Color(0, 123, 224);
    Color sea_deep_color = Color(194, 227, 255);
    float sea_level = -0.1f; // percentage of height under the sea

    // main colors
    Color sky_color = Color::White;
    Color terrain_color = Color::White;

    // under shadow
    float under_shadow_distance = 1500.f; // (negative value to hide)
	float h_threshold = -1.f; // remove terrain under a certain height (cool for islands)
 	bool show_outside_shadows = false; // show shadows outside of the terrain (on the white background)

    float alpha_shadows = 1.f; // transparency of the shadows (0 for no shadow, 1 for full shadow)
  };

  QImage PBR_Render(const PBR_Render_Options& options) const;

protected:
  virtual void paintGL();
  virtual void initializeGL();
  virtual void ReloadShaders();

public slots:
    virtual void keyPressEvent(QKeyEvent*);
};

