#ifdef __WIN32
  #define GLEW_STATIC
#endif

#include <iostream>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <lodepng/lodepng.h>
#include "Mat4x4/mat4x4.hpp"

const int n = 100; // grid size
const float far = 1000.f; // far plane
const float near = 0.01f; // near plane
const float PI = 3.14159F;
const float FOV_rad = 45.f / 180.f * PI; // 45 degrees
const std::string png_paths[2] = {".\\data\\cell.png", ".\\data\\dot.png"}; // paths to textures
const int textures_count = 2;
float aspect_ratio = 4.f / 3.f; // window aspect ratio
float scaling_ratio = 1.f; // zoom


GLFWwindow *g_window; // window descriptor

GLuint g_shaderProgram; // shader program descriptor
GLint g_uMVP; // Model View Projection descriptor
GLint g_uMV; // Model View descriptor
GLuint g_textures[textures_count]; // textures descriptor
GLuint mapLocs[textures_count]; // textures map location

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
	"layout(location = 1) in vec2 a_texture;" // texture coordinates
	"out vec3  v_pos, v_normal;"
	"out vec2 v_texCoord;" 
  // declaring matrix uniforms (MVP, MV, MN)
    "uniform mat4 u_mv, u_mvp;"
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
	"  v_normal = normalize(transpose(inverse(mat3(u_mv))) * grad_F);"
	"  v_pos = (u_mv * vec4(position, 1.f)).xyz;"
  // defining the gl_Position system variable
    "  gl_Position = u_mvp * vec4(position, 1.f);"
	"  v_texCoord = a_texture;"
    "}";

  const GLchar fsh[] =
    "#version 330\n"
  // explicitly declaring rendering target (color buffer) / output variable
	"layout(location = 0) out vec4 o_color;"
	"in vec3 v_pos, v_normal;"
	"in vec2 v_texCoord;"
	"uniform sampler2D u_map1, u_map2;"
    "void main() {"
  // Phong Shading
    "  vec3 normal = - normalize(v_normal);"
  	"  float d_min = 0.3f;"
	"  float s_focus = 4.f;"
	"  vec3 c_light = vec3(220.f, 220.f, 220.f) / 255.f;"
	"  vec3 c_base = vec3(53.f, 104.f, 103.f) / 255.f;"
	"  vec3 L = vec3(10.f, 10.f, 0.f);"
	"  vec3 E = vec3(0.f, 0.f, 0.f);"

	"  vec3 l = normalize(v_pos - L);"
	"  float cos_a = dot(-l, normal);"
	"  float d = max(cos_a, d_min);"
	"  vec3 e = normalize(E - v_pos);"
	"  vec3 r = reflect(l, normal);"
	
	"  float s = cos_a > 0.f ? max(pow(dot(r,e), s_focus), 0.f) : 0.f;"
  // Mix two textures
    "  vec4 mixed_textures = mix(texture(u_map1, v_texCoord), texture(u_map2, v_texCoord), 0.65);"
	"  o_color = vec4(c_light * (d * mixed_textures.xyz + vec3(s)), 1.f);"
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
  // Vertex array of n^2 elements (4 attributes for each vertex)
  GLfloat *vertices = new GLfloat[n * n * 4];
  // Index array
  GLuint *indices = new GLuint[(n - 1) * (n - 1) * 6];
  uint32_t current_v = 0;
  for (int z = 0; z < n; ++z) {
    for (int x = 0; x < n; ++x) {
      vertices[current_v++] = (float)x / n - 0.5f;
      vertices[current_v++] = (float)z / n - 0.5f;
      vertices[current_v++] = (float)x / 10;
      vertices[current_v++] = (float)z / 10;
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
  glBufferData(GL_ARRAY_BUFFER, n * n * 4 * sizeof(GLfloat), vertices,
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
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (const GLvoid *)0);
  // Allows using data buffer for attribute 1 (a_texture)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (const GLvoid *)(2 * sizeof(GLfloat)));

  return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool createTextures(const std::string *filenames) {
  // Creates texture object
  glGenTextures(textures_count, g_textures);
  std::vector<unsigned char> png;
  std::vector<unsigned char> image;
  GLuint texW, texH;
  for (int i = 0; i < textures_count; ++i) {
    // Activate texture object
    glBindTexture(GL_TEXTURE_2D, g_textures[i]);

    // Load PNG file from disk to memory first, then decode to raw pixels in memory
    GLuint error = lodepng::load_file(png, filenames[i]);
    if (!error) error = lodepng::decode(image, texW, texH, png);
    if (error) { 
	  std::cout << "decoder error" << error << ": " << lodepng_error_text(error) << std::endl;
  	  return 0;
    }
    png.clear();
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Anisotropic filtering
    if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
	  GLfloat fLargest;
	  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    }
    // Load texture into VRAM
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    image.clear();
    // Generate MIP
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  mapLocs[0] = glGetUniformLocation(g_shaderProgram, "u_map1");
  mapLocs[1] = glGetUniformLocation(g_shaderProgram, "u_map2");
  return 1;
}

bool init() {
  // Set initial color of color buffer to white.
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  return createShaderProgram() && createModel() && createTextures(png_paths);
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
  
  // Sending texture to the pipeline
  for (GLuint i = 0; i < textures_count; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, g_textures[i]);
    glUniform1i(mapLocs[i], i);
  }
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
  glDeleteTextures(textures_count, g_textures);
}


int main() {
  
  Mat4x4 T = Mat4x4::get_translation_mat(Vec3(0.f,0.f,-5.f));
  Vec3 v(1.f, 1.f, 1.f);

  // Initialize OpenGL
  if (!initOpenGL()) return -1;

  // Initialize graphical resources.
  bool isIninialised = init();

  if (isIninialised) {
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
