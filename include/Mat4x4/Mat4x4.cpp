#include "Mat4x4.hpp"
#include <cmath>
#include <iostream>

Mat4x4::Mat4x4() {
  m_mat[0] = 1.f, m_mat[1] = 0.f, m_mat[2] = 0.f, m_mat[3] = 0.f;
  m_mat[4] = 0.f, m_mat[5] = 1.f, m_mat[6] = 0.f, m_mat[7] = 0.f;
  m_mat[8] = 0.f, m_mat[9] = 0.f, m_mat[10] = 1.f, m_mat[11] = 0.f;
  m_mat[12] = 0.f, m_mat[13] = 0.f, m_mat[14] = 0.f, m_mat[15] = 1.f;
}

Mat4x4::Mat4x4(const float diag_num) {
  m_mat[0] = diag_num, m_mat[1] = 0.f, m_mat[2] = 0.f, m_mat[3] = 0.f;
  m_mat[4] = 0.f, m_mat[5] = diag_num, m_mat[6] = 0.f, m_mat[7] = 0.f;
  m_mat[8] = 0.f, m_mat[9] = 0.f, m_mat[10] = diag_num, m_mat[11] = 0.f;
  m_mat[12] = 0.f, m_mat[13] = 0.f, m_mat[14] = 0.f, m_mat[15] = diag_num;
}

Mat4x4::Mat4x4(const Mat4x4 &another) {
  for (int i = 0; i < 16; ++i) {
    m_mat[i] = another.m_mat[i];
  }
}

Mat4x4::Mat4x4(const float *mat_ptr) {
  for (int i = 0; i < 16; ++i) {
    m_mat[i] = mat_ptr[i];
  }
}

Mat4x4 Mat4x4::transpose()
{
  auto old_mat = this->ptr();
  float new_mat[16];
  for (int i = 0; i < m_size; ++i)
    for (int j = 0; j < m_size; ++j)
      new_mat[i*m_size+j] = old_mat[j*m_size+i];
  return Mat4x4(new_mat);
}

Mat4x4 Mat4x4::get_scaling_mat(const Vec3 &v_xyz){
  float scaling_mat[16] {
      v_xyz.x, 0.f, 0.f, 0.f,
    0.f,   v_xyz.y, 0.f, 0.f,
    0.f, 0.f,   v_xyz.z, 0.f,
    0.f, 0.f, 0.f, 1.f };
  return Mat4x4(scaling_mat);
}

Mat4x4 Mat4x4::get_translation_mat(const Vec3 &v_xyz){
  float translation_mat[16] {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    v_xyz.x,   v_xyz.y,   v_xyz.z,   1.f };
  return Mat4x4(translation_mat);
}

Mat4x4 Mat4x4::get_rotation_mat(const Vec3 &v_xyz_normalised, const float theta_rad){
  float c = cosf(theta_rad);
  float s = sinf(theta_rad);
  float x = v_xyz_normalised.x;
  float y = v_xyz_normalised.y;
  float z = v_xyz_normalised.z;
  float rotation_mat[16] {
    x * x * (1.f - c) + c, y * x * (1.f - c) + z * s, x * z * (1.f - c) - y * s, 0.f,
    x * y * (1.f - c) - z * s, y * y * (1.f - c) + c, y * z * (1.f - c) + x * s, 0.f,
    x * z * (1.f - c) + y * s, y * z * (1.f - c) - x * s, z * z * (1.f - c) + c, 0.f,
    0.f, 0.f, 0.f, 1.f
  };
  return Mat4x4(rotation_mat);
}

Mat4x4 Mat4x4::look_at(Vec3 eye, Vec3 target, Vec3 up){
  Vec3 zaxis = Vec3::normalise(eye - target); // The "forward" vector.
  Vec3 xaxis = Vec3::normalise(Vec3::cross(up, zaxis)); // The "right" vector.
  Vec3 yaxis = Vec3::cross(zaxis, xaxis); // The "up" vector.
  // Create a 4x4 view matrix from the right, up, forward and eye position vectors
  float viewMatrix[16]{
    xaxis.x, xaxis.y, xaxis.z, -Vec3::dot(xaxis,eye),
    yaxis.x, yaxis.y, yaxis.z, -Vec3::dot(yaxis, eye),
    zaxis.x, zaxis.y, zaxis.z, -Vec3::dot(zaxis, eye),
    0.f, 0.f, 0.f, 1.f};
  return Mat4x4(viewMatrix);
}

Mat4x4 Mat4x4::get_perspective_proj_mat(float near, float far, float aspect, float FOV_rad)  {
  float tan = tanf(FOV_rad / 2.0f);
  float persp_proj_mat[16] = {
    1.f / (aspect * tan), 0.f, 0.f, 0.f,
    0.f, 1 / tan, 0.f, 0.f,
    0.f, 0.f, - (far + near) / (far - near), -1.f,
    0.f, 0.f, -2.f * far * near / (far - near), 0.f
  };
  return Mat4x4(persp_proj_mat);
}

Mat4x4 Mat4x4::get_parallel_proj_mat(float near, float far, float aspect, float FOV_rad)  {
  float tan = tanf(FOV_rad / 2.0f);
  float paral_proj_mat[16] = {
    1.f / (near * tan), 0.f, 0.f, 0.f,
    0.f, 1.f / (near * aspect * tan), 0.f, 0.f,
    0.f, 0.f, -2.f / (far - near), -1.f,
    0.f, 0.f, - (far + near) / (far - near), 1.f
  };
  return Mat4x4(paral_proj_mat);
}

void Mat4x4::get_ptr_mat3x3(const Mat4x4 &m_mat, float* mat3x3) {
    auto ptr_mat3x3 = m_mat.ptr();
    mat3x3[0] = ptr_mat3x3[0];
    mat3x3[1] = ptr_mat3x3[1];
    mat3x3[2] = ptr_mat3x3[2];
    mat3x3[3] = ptr_mat3x3[4];
    mat3x3[4] = ptr_mat3x3[5];
    mat3x3[5] = ptr_mat3x3[6];
    mat3x3[6] = ptr_mat3x3[8];
    mat3x3[7] = ptr_mat3x3[9];
    mat3x3[8] = ptr_mat3x3[10];
}

void Mat4x4::Print() {
  for(int i = 0; i<4; ++i) {
	  for(int j = 0; j<4; ++j)
        std::cout << this->ptr()[i * 4 + j] << " ";
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
