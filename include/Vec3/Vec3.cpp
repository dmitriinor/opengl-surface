#include "Vec3.hpp"

Vec3::Vec3(const float _x, const float _y, const float _z) {
  x = _x;
  y = _y;
  z = _z;
}

float Vec3::len(const Vec3 &vec) {
  return 1.f / inverse_root(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float Vec3::dot(const Vec3 &v1, const Vec3 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

Vec3 Vec3::cross(const Vec3 &v1, const Vec3 &v2) {
  return Vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
              v1.x * v2.y - v1.y * v2.x);
}

Vec3 Vec3::normalise(Vec3 vec) { return vec / len(vec); }

void Vec3::set_xyz(const float value) { this -> x = value, this -> y = value, this -> z = value; }

static float inverse_root(const float number) {
  long i;
  float x2, y;
  const float threehalfs = 1.5f;

  x2 = number * 0.5f;
  y = number;
  i = *(long*)&y;
  i = 0x5f3759df - (i >> 1);
  y = *(float*)&i;
  y = y * (threehalfs - (x2 * y * y));  // 1st iteration
  y = y * (threehalfs - (x2 * y * y));  // 2nd iteration, this can be removed
  return y;
}