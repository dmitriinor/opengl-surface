#ifdef __WIN32
  #define GLEW_STATIC
#endif

#include <iostream>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "Mat4x4/mat4x4.hpp"

constexpr int n = 100; // grid size
constexpr float far = 1000.f; // far plane
constexpr float near = 0.01f; // near plane
constexpr float PI = 3.14159F;
constexpr float FOV_rad = 45.f / 180.f * PI; // 45 degrees
float aspect_ratio = 4.f / 3.f; // window aspect ratio
float scaling_ratio = 1.f; // zoom

GLFWwindow *g_window; // window descriptor

GLuint g_shaderProgram; // shader program descriptor
GLint g_uMVP;
GLint g_uMV;

struct Model {
  GLuint vbo; // vertex buffer object descriptor
  GLuint ibo; // index buffer object descriptor
  GLuint vao; // vertex array object descriptor
  GLsizei indexCount; // number of indices
};

Model g_model;

GLuint createShader(const GLchar *code, GLenum type) {
  // creating shader object
  GLuint result = glCreateShader(type);
  // loading shader source code
  glShaderSource(result, 1, &code, NULL);
  glCompileShader(result);

  GLint compiled;
  // requesting compiler status
  glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);
  // logging error message & deleting shader if not compiled
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 0) {
      char infoLog[infoLen];
      glGetShaderInfoLog(result, infoLen, NULL, infoLog);
      std::cout << "Shader compilation error" << std::endl
                << infoLog << std::endl;
    }
    glDeleteShader(result);
    return 0;
  }
  // compiled shader descriptor
  return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh) {
  // creating shader program object
  GLuint result = glCreateProgram();
  // attaching vector & fragment shaders to program
  glAttachShader(result, vsh);
  glAttachShader(result, fsh);

  glLinkProgram(result);

  GLint linked;
  // requesting linker status
  glGetProgramiv(result, GL_LINK_STATUS, &linked);
  // logging errors & deleting program object if not linked
  if (!linked) {
    GLint infoLen = 0;
    glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 0) {
      char *infoLog = (char *)alloca(infoLen);
      glGetProgramInfoLog(result, infoLen, NULL, infoLog);
      std::cout << "Shader program linking error" << std::endl
                << infoLog << std::endl;
    }
    glDeleteProgram(result);
    return 0;
  }

  return result;
}

bool createShaderProgram() {
  g_shaderProgram = 0;

  const GLchar vsh[] =
  // setting GLSL version
    "#version 330\n"
  // declaring the attributes (vertices data) & assigning the descriptors to them
    "layout(location = 0) in vec2 a_position;" // x and y vector 
    "layout(location = 1) in vec3 a_color;" // r, g, b vector
  // declaring matrix uniforms (MVP, MV, MN)
    "uniform mat4 u_mv, u_mvp;"
	
	"out vec3  v_pos, v_normal;"
  // declaring and defining surface function and derivatives
    "float a = 0.8f;"
	"float b = 0.6f;"
    "float f_surface (float x, float y) { return (x*x/(a*a) - y*y/(b*b)); }"
	"float dF_dx (float x, float y) { return 2*x/(a*a);}"
	"float dF_dy (float x, float y) { return -2*y/(b*b);}"
	"float dF_dz () { return -1.f; }"
    "void main(){"
  // defining position vector
    "  vec3 position = vec3(a_position[0], a_position[1], f_surface(a_position[0], a_position[1]));"
	"  vec3 grad_F = vec3(dF_dx(a_position[0], a_position[1]),"
	"                     dF_dy(a_position[0], a_position[1]),"
	"                     dF_dz());"
  // normal transformation
	"  v_normal = transpose(inverse(mat3(u_mv))) * grad_F;"
	"  v_pos = (u_mv * vec4(position, 1.f)).xyz;"
  // defining the gl_Position system variable
    "  gl_Position = u_mvp * vec4(position, 1.f);"
    "}";

  const GLchar fsh[] =
    "#version 330\n"
  // explicitly declaring rendering target (color buffer) / output variable
	"layout(location = 0) out vec4 o_color;"
	"in vec3 v_pos, v_normal;"
    "uniform mat4 u_mv;"
    "void main() {"
  // Phong Shading
    "  vec3 normal = - normalize(v_normal);"
  	"  float d_min = 0.2f;"
	"  float s_focus = 5.f;"
	"  vec3 c_light = vec3(245.f, 193.f, 253.f) / 255.f;"
	"  vec3 c_base = vec3(53.f, 104.f, 103.f) / 255.f;"
	"  vec3 L = vec3(10.f, 10.f, 10.f);"
	"  vec3 E = vec3(0.f, 0.f, 0.f);"

	"  vec3 l = normalize(v_pos - L);"
	"  float cos_a = dot(-l, normal);"
	"  float d = max(cos_a, d_min);"
	"  vec3 e = normalize(E - v_pos);"
	"  vec3 r = reflect(l, normal);"
	
	"  float s = cos_a > 0.f ? max(pow(dot(r,e), s_focus), 0.f) : 0.f;"
	"  o_color = vec4(c_light * (d * c_base + vec3(s)), 1.f);"
    "}";

  auto vertexShader = createShader(vsh, GL_VERTEX_SHADER);
  auto fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

  g_shaderProgram = createProgram(vertexShader, fragmentShader);
  
  // getting the descriptor to the MVP uniform
  g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");
  g_uMV = glGetUniformLocation(g_shaderProgram, "u_mv");

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return g_shaderProgram != 0;
}

bool createModel() {
  // Vertex array of n^2 elements (5 attributes for each vertex)
  GLfloat *vertices = new GLfloat[n * n * 5];
  // Index array
  GLuint *indices = new GLuint[(n - 1) * (n - 1) * 6];
  uint32_t current_v = 0;
  for (int z = 0; z < n; ++z) {
    for (int x = 0; x < n; ++x) {
      vertices[current_v++] = (float)x / n - 0.5f;
      vertices[current_v++] = (float)z / n - 0.5f;
      vertices[current_v++] = 0.f;
      vertices[current_v++] = 1.f;
      vertices[current_v++] = 0.f;
    }
  }
  // Going through cells counter-clockwise
  current_v = 0;
  for (int z = 0; z < n - 1; ++z) {
    for (int x = 0; x < n - 1; ++x) {
      indices[current_v++] = z * n + x;
      indices[current_v++] = (z + 1) * n + x;
      indices[current_v++] = (z + 1) * n + x + 1;
      indices[current_v++] = z * n + x + 1;
      indices[current_v++] = z * n + x;
      indices[current_v++] = (z + 1) * n + x + 1;
    }
  }
  // Generates 1 Vertex Array Object and stores it in Model object's vao field
  glGenVertexArrays(1, &g_model.vao);
  // Activates VAO
  glBindVertexArray(g_model.vao);
  // Generates 1 Vertex Buffer Object and stores it in Model object's vbo field
  glGenBuffers(1, &g_model.vbo);
  // Activates VBO
  glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
  // Copying vbo data to active vertex buffer
  glBufferData(GL_ARRAY_BUFFER, n * n * 5 * sizeof(GLfloat), vertices,
               GL_STATIC_DRAW);
  delete[] vertices;
  // Generates 1 Index Buffer Object and stores it in Model object's ibo field
  glGenBuffers(1, &g_model.ibo);
  // Activates IBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
  // Copying ibo data to active index buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (n - 1) * (n - 1) * 6 * sizeof(GLuint),
               indices, GL_STATIC_DRAW);
  delete[] indices;

  g_model.indexCount = (n - 1) * (n - 1) * 6;
  // Allows using data buffer for attribute 0 (a_vertex)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        (const GLvoid *)0);
 // Allows using data buffer for attribute 1 (a_color)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                        (const GLvoid *)(2 * sizeof(GLfloat)));

  return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init() {
  // Set initial color of color buffer to white.
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  return createShaderProgram() && createModel();
}
  
void reshape(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  aspect_ratio = (float)width / (float)height;
}

void draw(Mat4x4 &T, Vec3 &v) {
  // Clears color and depth buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.117f, 0.117f, 0.176f, 1.f);
  // Activates shader program
  glUseProgram(g_shaderProgram);
  // Activates vao
  glBindVertexArray(g_model.vao);

  // X-Rotation & Y-rotation
  auto Rx = Mat4x4::get_rotation_mat(Vec3(1.f, 0.f, 0.f), - PI / 1.75f);
  auto Ry = Mat4x4::get_rotation_mat(Vec3(0.f, 1.f, 0.f), (float)glfwGetTime() * PI / 2.f);

  // Scaling
  v.set_xyz(scaling_ratio);
  auto S = Mat4x4::get_scaling_mat(v);
  
  // Projection
  auto P = Mat4x4::get_perspective_proj_mat(near, far, aspect_ratio, FOV_rad);

  // Building MVP matrix 
  auto MV = T * S * Ry * Rx;
  auto MVP = P * MV;
  
  // Sending to the shader
  glUniformMatrix4fv(g_uMV, 1, GL_FALSE, MV.ptr());
  glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, MVP.ptr());
  // Draw call itself (sending to the pipeline)
  glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {      
    if (key == GLFW_KEY_UP && action == GLFW_PRESS){
        scaling_ratio += 0.2f;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        scaling_ratio -= 0.2f;
    }
}

bool initOpenGL() {
  // Initialize GLFW functions.
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return false;
  }

  // Request OpenGL 3.3 without obsoleted functions.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create window.
  g_window = glfwCreateWindow(800, 600, "OpenGL Surface", NULL, NULL);
  if (g_window == NULL) {
    std::cout << "Failed to open GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }

  // Initialize OpenGL context with.
  glfwMakeContextCurrent(g_window);

  // Set internal GLEW variable to activate OpenGL core profile.
  glewExperimental = true;

  // Initialize GLEW functions.
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return false;
  }

  // Ensure we can capture the escape key being pressed.
  glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_FALSE);

  // Set callback for framebuffer resizing event.
  glfwSetFramebufferSizeCallback(g_window, reshape);
  
  // Set callback for keys pressed event.
  glfwSetKeyCallback(g_window, key_callback);

  return true;
}

void tearDownOpenGL() {
  // Terminate GLFW.
  glfwTerminate();
}

void cleanup() {
  if (g_shaderProgram != 0) glDeleteProgram(g_shaderProgram);
  if (g_model.vbo != 0) glDeleteBuffers(1, &g_model.vbo);
  if (g_model.ibo != 0) glDeleteBuffers(1, &g_model.ibo);
  if (g_model.vao != 0) glDeleteVertexArrays(1, &g_model.vao);
}


int main() {
  
  Mat4x4 T = Mat4x4::get_translation_mat(Vec3(0.f,0.f,-5.f));
  Vec3 v(1.f, 1.f, 1.f);

  // Initialize OpenGL
  if (!initOpenGL()) return -1;
  
  // Initialize graphical resources.
  bool isIninialised = init();

  if (isIninialised) {
    //glEnable(GL_CULL_FACE);
    // Main loop until window closed or escape pressed.
    while (glfwWindowShouldClose(g_window) == 0) {
      // Draw Call.
      draw(T, v);

      // Swap buffers.
      glfwSwapBuffers(g_window);
      // Poll window events.
      glfwPollEvents();
      
    }
  }

  // Cleanup graphical resources.
  cleanup();

  // Tear down OpenGL.
  tearDownOpenGL();

  return isIninialised ? 0 : -1;
}
