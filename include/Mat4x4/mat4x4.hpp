#pragma once
#include "../Vec3/Vec3.hpp"
#define m_size 4

class Mat4x4
{
public:
  // default constructor creates identity matrix
  Mat4x4();
  // diag_elem on main diagonal, and zeros elsewhere
  Mat4x4(const float diag_elem);
  // copy constructor
  Mat4x4(const Mat4x4 &another);
  // creates matrix from array of 16 elements, e.g. mat.ptr()
  Mat4x4(const float *mat_ptr);
  // returns pointer to m_mat
  const float *ptr() const { return m_mat; }
  // matrix multiplication
  Mat4x4 operator*(const Mat4x4 &another) {
    float result_mat[16];
	for (int i = 0; i < 16; ++i) result_mat[i] = 0;
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            for(int k = 0; k < 4; ++k)
                result_mat[(i << 2) + j] += this->ptr()[(k  << 2) + j] * another.ptr()[(i << 2) + k];
    
    return Mat4x4(result_mat);
  }
  // matrix multiplication
  Mat4x4 operator*(const float *mat_ptr) {
    float result_mat[16];	
	for (int i = 0; i < 16; ++i) result_mat[i] = 0;
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            for(int k = 0; k < 4; ++k)
                result_mat[(i << 2) + j] += this->ptr()[(k << 2) + j] * mat_ptr[(i << 2) + k];
    
    return Mat4x4(result_mat);
  }
  //returns an inversed copy of Mat4x4
  Mat4x4 inverse();
  //returns a transposed copy of Mat4x4
  Mat4x4 transpose();
  //printing
  void Print();
  //returns dot product of scaling matrix and provided vector (x,y,z)
  static Mat4x4 get_scaling_mat(const Vec3 &v_xyz);
  //returns dot product of translation matrix and provided vector (x,y,z)
  static Mat4x4 get_translation_mat(const Vec3 &v_xyz);
  //returns dot product of rotation matrix and normalised provided vector (x,y,z)
  static Mat4x4 get_rotation_mat(const Vec3 &v_xyz_normalised, const float theta_rad);

  static void get_ptr_mat3x3(const Mat4x4 &m_mat, float* mat3x3);
  
  static Mat4x4 look_at(Vec3 eye, Vec3 target, Vec3 up);

  static Mat4x4 get_perspective_proj_mat(const float near, const float far, const float aspect, const float FOV_rad);

  static Mat4x4 get_parallel_proj_mat(const float near, const float far, const float aspect, const float FOV_rad);

private:
  float m_mat[16];
};
