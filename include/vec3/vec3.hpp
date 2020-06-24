#pragma once

struct Vec3 {
  float x, y, z;

  Vec3(const float _x, const float _y, const float _z);
  
  void set_xyz(const float value);

  static float len(const Vec3 &vec);

  Vec3 operator -(const Vec3 &vec) { return Vec3(x - vec.x, y - vec.y, z - vec.z); }

  Vec3 operator /(const float a) { return Vec3(x / a, y / a, z / a); }
  
  Vec3 operator *(const float a) { return Vec3(x * a, y * a, z * a); }

  static float dot(const Vec3 &v1, const Vec3 &v2);

  static Vec3 cross(const Vec3 &v1, const Vec3 &v2);

  static Vec3 normalise(Vec3 vec);
};

static float inverse_root(const float number);