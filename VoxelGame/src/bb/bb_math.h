// simple single file math library created by Brajan Bartoszewicz
// feel free to you use it in your projects
#ifndef BB_MATH_H_
#define BB_MATH_H_

#include "bb_types.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define bb_abs(x) ((x) > 0 ? (x) : (-x))

static r32 bb_toRadiansReal32(r32 degrees) { return (r32)(degrees * (M_PI / 180.0f)); }
static r32 bb_toDegreesReal32(r32 radians) { return (r32)(radians * (180.0f / M_PI)); }

struct bbVec2 {
  union {
    struct { r32 x, y; };
    struct { r32 u, v; };
    struct { r32 s, t; };
  };

  bbVec2(r32 x = 0.0f, r32 y = 0.0f) : x(x), y(y) {}

  bbVec2 operator+(const bbVec2& rhs) const { return{ x + rhs.x, y + rhs.y }; }
  bbVec2 operator-(const bbVec2& rhs) const { return{ x - rhs.x, y - rhs.y }; }
  bbVec2 operator*(const bbVec2& rhs) const { return{ x * rhs.x, y * rhs.y }; }
  bbVec2 operator/(const bbVec2& rhs) const { return{ x / rhs.x, y / rhs.y }; }
  bbVec2 operator*(r32 scalar) const { return{ x * scalar, y * scalar }; }
  bbVec2& operator+=(const bbVec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
  bbVec2& operator-=(const bbVec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
  bbVec2& operator*=(const bbVec2& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
  bbVec2& operator/=(const bbVec2& rhs) { x /= rhs.x; y /= rhs.y; return *this; }
  bool operator==(const bbVec2& rhs) const { return (x == rhs.x && y == rhs.y); }
  bool operator!=(const bbVec2& rhs) const { return !(*this == rhs); }
};

struct bbVec3 {
  union {
    struct { r32 x, y, z; };
    struct { r32 r, g, b; };
  };

  bbVec3(r32 x = 0.0f, r32 y = 0.0f, r32 z = 0.0f) : x(x), y(y), z(z) { }

  bbVec3 operator+(const bbVec3& rhs) const { return{ x + rhs.x, y + rhs.y, z + rhs.z }; }
  bbVec3 operator-(const bbVec3& rhs) const { return{ x - rhs.x, y - rhs.y, z - rhs.z }; }
  bbVec3 operator*(const bbVec3& rhs) const { return{ x * rhs.x, y * rhs.y, z * rhs.z }; }
  bbVec3 operator/(const bbVec3& rhs) const { return{ x / rhs.x, y / rhs.y, z / rhs.z }; }
  bbVec3 operator*(r32 scalar) const { return{ x * scalar, y * scalar, z * scalar }; }
  bbVec3& operator+=(const bbVec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
  bbVec3& operator-=(const bbVec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
  bbVec3& operator*=(const bbVec3& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
  bbVec3& operator/=(const bbVec3& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }
  bool operator==(const bbVec3& rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }
  bool operator!=(const bbVec3& rhs) const { return !(*this == rhs); }
};

struct bbVec4 {
  union {
    struct { r32 x, y, z, w; };
    struct { r32 r, g, b, a; };
    bbVec3 xyz;
    bbVec3 rgb;
    bbVec2 xy;
  };

  bbVec4(r32 x = 0.0f, r32 y = 0.0f, r32 z = 0.0f, r32 w = 0.0f) : x(x), y(y), z(z), w(w) { }
};

struct bbQuaternion {
  union {
    bbVec4 xyzw;
    struct { r32 x, y, z, w; };
  };

  bbQuaternion(r32 x = 0.0f, r32 y = 0.0f, r32 z = 0.0f, r32 w = 1.0f) : x(x), y(y), z(z), w(w) { }

  bbQuaternion(r32 angle, const bbVec3& axis) {
    r32 sin_half_angle = sinf(bb_toRadiansReal32(angle) / 2.0f);
    r32 cos_half_angle = cosf(bb_toRadiansReal32(angle) / 2.0f);

    x = axis.x * sin_half_angle;
    y = axis.y * sin_half_angle;
    z = axis.z * sin_half_angle;
    w = cos_half_angle;
  }

  bbQuaternion(const bbVec3& angles) {
    bbQuaternion result = bbQuaternion(angles.x, { 1, 0, 0 }) * bbQuaternion(angles.y, { 0, 1, 0 }) * bbQuaternion(angles.z, { 0, 0, 1 });

    x = result.x;
    y = result.y;
    z = result.z;
    w = result.w;
  }

  bbQuaternion operator*(const bbQuaternion& v) const {
    return {
            (x * v.w) + (w * v.x) + (y * v.z) - (z * v.y),
            (y * v.w) + (w * v.y) + (z * v.x) - (x * v.z),
            (z * v.w) + (w * v.z) + (x * v.y) - (y * v.x),
            (w * v.w) - (x * v.x) - (y * v.y) - (z * v.z)
           };
  }

  bbQuaternion operator*(const bbVec3& v) const {
    return {
            (w * v.x) + (y * v.z) - (z * v.y),
            (w * v.y) + (z * v.x) - (x * v.z),
            (w * v.z) + (x * v.y) - (y * v.x),
            -(x * v.x) - (y * v.y) - (z * v.z)
           };
  }
};

struct bbMat4 {
  r32 m[4][4];

  bbMat4(r32 diagonal = 1.0f) {
    for (uint32_t i = 0; i < 4; i++) {
      for (uint32_t j = 0; j < 4; j++) {
        if (i == j) {
          m[i][j] = diagonal;
        } else {
          m[i][j] = 0.0f;
        }
      }
    }
  }

  bbMat4 operator*(const bbMat4& rhs) const {
    bbMat4 result;
    for (uint32_t i = 0; i < 4; i++) {
      for (uint32_t j = 0; j < 4; j++) {
        result[i][j] = 
          m[i][0] * rhs[0][j] +
          m[i][1] * rhs[1][j] +
          m[i][2] * rhs[2][j] +
          m[i][3] * rhs[3][j];
      }
    }

    return result;
  }

  const r32* operator[](uint32_t index) const { return m[index]; }
  r32* operator[](uint32_t index) { return m[index]; }
};

static bbQuaternion bb_initQuaternion(r32 angle, const bbVec3& axis) {
  r32 sin_half_angle = sinf(bb_toRadiansReal32(angle) / 2.0f);
  r32 cos_half_angle = cosf(bb_toRadiansReal32(angle) / 2.0f);

  return { axis.x * sin_half_angle, axis.y * sin_half_angle, axis.z * sin_half_angle, cos_half_angle };
}

static bbQuaternion bb_initQuaternion(const bbVec3& angles) {
  bbQuaternion result = bbQuaternion(angles.x, { 1, 0, 0 }) * bbQuaternion(angles.y, { 0, 1, 0 }) * bbQuaternion(angles.z, { 0, 0, 1 });
  return result;
}

static r32 bb_dot(const bbVec2& a, const bbVec2& b) { return a.x * b.x + a.y * b.y; }
static r32 bb_length(const bbVec2& v) { return sqrt(bb_dot(v, v)); }
static r32 bb_dot(const bbVec3& a, const bbVec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static r32 bb_length(const bbVec3& v) { return sqrt(bb_dot(v, v)); }
static bbVec3 bb_normalize(const bbVec3& v) { r32 l = bb_length(v); return{ v.x / l, v.y / l, v.z / l }; }
static bbVec3 bb_cross(const bbVec3& a, const bbVec3& b) {
  return bbVec3(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  );
}

static bbQuaternion bb_conjugate(const bbQuaternion& v);
static bbVec3 bb_rotate(const bbVec3& v, const bbQuaternion& q) {
  bbQuaternion conjugate = bb_conjugate(q);
  bbQuaternion w = q * v * conjugate;

  return { w.x, w.y, w.z };
}

static r32 bb_length(const bbQuaternion& v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
static bbQuaternion bb_normalize(const bbQuaternion& v) { r32 l = bb_length(v); return{ v.x / l, v.y / l, v.z / l, v.w / l }; }
static bbQuaternion bb_conjugate(const bbQuaternion& v) { return{ -v.x, -v.y, -v.z, v.w }; }
static bbVec3 bb_forward(const bbQuaternion& v) { return bb_rotate({0, 0, 1}, v); }
static bbVec3 bb_back(const bbQuaternion& v) { return bb_rotate({ 0, 0, -1 }, v); }
static bbVec3 bb_up(const bbQuaternion& v) { return bb_rotate({ 0, 1, 0 }, v); }
static bbVec3 bb_down(const bbQuaternion& v) { return bb_rotate({ 0, -1, 0 }, v); }
static bbVec3 bb_right(const bbQuaternion& v) { return bb_rotate({ 1, 0, 0 }, v); }
static bbVec3 bb_left(const bbQuaternion& v) { return bb_rotate({ -1, 0, 0 }, v); }

static bbMat4 bb_orthographic(r32 left, r32 right, r32 bottom, r32 top, r32 near_z, r32 far_z) {
  bbMat4 ret(1.0f);

  ret[0][0] = 2.0f / (right - left);
  ret[1][1] = 2.0f / (top - bottom);
  ret[2][2] = 2.0f / (far_z - near_z);

  ret[0][3] = -(right + left) / (right - left);
  ret[1][3] = -(top + bottom) / (top - bottom);
  ret[2][3] = -(far_z + near_z) / (far_z - near_z);
  return ret;
}

static bbMat4 bb_perspective(r32 fov, r32 aspect, r32 near_z, r32 far_z) {
  bbMat4 ret(1.0f);

  r32 range = near_z - far_z;
  r32 tan_half_fov = tanf(fov / 2.0f);

  ret[0][0] = 1.0f / (tan_half_fov * aspect);
  ret[1][1] = 1.0f / tan_half_fov;
  ret[2][2] = (-near_z - far_z) / range;
  ret[2][3] = 2.0f * far_z * near_z / range;
  ret[3][3] = 0.0f;
  ret[3][2] = 1.0f;

  return ret;
}

static bbMat4 bb_translate(const bbVec3& v) {
  bbMat4 mat(1.0f);
  mat[0][3] = v.x;
  mat[1][3] = v.y;
  mat[2][3] = v.z;
  return mat;
}

static bbMat4 bb_scale(const bbVec3& v) {
  bbMat4 mat(1.0f);
  mat[0][0] = v.x;
  mat[1][1] = v.y;
  mat[2][2] = v.z;
  return mat;
}

static bbMat4 bb_rotate(const bbVec3& v) {
  bbMat4 rx(1.0f);
  bbMat4 ry(1.0f);
  bbMat4 rz(1.0f);

  r32 x = bb_toRadiansReal32(v.x);
  r32 y = bb_toRadiansReal32(v.y);
  r32 z = bb_toRadiansReal32(v.z);

  rx[1][1] = cosf(x);
  rx[2][1] = sinf(x);
  rx[1][2] = -sinf(x);
  rx[2][2] = cosf(x);

  ry[0][0] = cosf(y);
  ry[2][0] = sinf(y);
  ry[0][2] = -sinf(y);
  ry[2][2] = cosf(y);

  rz[0][0] = cosf(z);
  rz[1][0] = sinf(z);
  rz[0][1] = -sinf(z);
  rz[1][1] = cosf(z);

  return rz * ry * rx;
}

static bbMat4 bb_rotate(r32 angle, const bbVec3& axis) {
  bbMat4 result(1.0f);

  r32 r = bb_toRadiansReal32(angle);
  r32 c = cos(r);
  r32 s = sin(r);
  r32 omc = 1.0f - c;

  r32 x = axis.x;
  r32 y = axis.y;
  r32 z = axis.z;

  result[0][0] = x * omc + c;
  result[1][0] = y * x * omc + z * s;
  result[2][0] = x * z * omc - y * s;

  result[0][1] = x * y * omc - z * s;
  result[1][1] = y * omc + c;
  result[2][1] = y * z * omc + x * s;

  result[0][2] = x * z * omc + y * s;
  result[1][2] = y * z * omc - x * s;
  result[2][2] = z * omc + c;
  return result;
}

static bbMat4 bb_rotate(const bbVec3& n, const bbVec3& v, const bbVec3& u) {
  bbMat4 result(1.0f);

  result[0][0] = u.x;
  result[1][0] = v.x;
  result[2][0] = n.x;

  result[0][1] = u.y;
  result[1][1] = v.y;
  result[2][1] = n.y;

  result[0][2] = u.z;
  result[1][2] = v.z;
  result[2][2] = n.z;
  return result;
}

static bbMat4 Rotate(const bbVec3& forward, const bbVec3& up) {
  bbVec3 n = bb_normalize(forward);
  bbVec3 u = bb_cross(bb_normalize(up), n);
  bbVec3 v = bb_cross(n, u);
  return bb_rotate(n, v, u);
}

static bbMat4 bb_rotate(const bbQuaternion& q) {
  bbVec3 forward = bbVec3(2.0f * (q.x * q.z - q.w * q.y), 2.0f * (q.y * q.z + q.w * q.x), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
  bbVec3 up      = bbVec3(2.0f * (q.x * q.y + q.w * q.z), 1.0f - 2.0f * (q.x * q.x + q.z * q.z), 2.0f * (q.y * q.z - q.w * q.x));
  bbVec3 right   = bbVec3(1.0f - 2.0f * (q.y * q.y + q.z * q.z), 2.0f * (q.x * q.y - q.w * q.z), 2.0f * (q.x * q.z + q.w * q.y));
  return bb_rotate(forward, up, right);
}

static bbVec3 bb_transform(const bbMat4& m, const bbVec3& v) {
  return bbVec3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
}

static r32 bb_clamp(r32 value, r32 min, r32 max) {
  if (value > max)
    return max;
  else if (value < min)
    return min;
  return value;
}

static r32 bb_max(r32 a, r32 b) {
  return a > b ? a : b;
}

static r32 bb_min(r32 a, r32 b) {
  return a > b ? b : a;
}

#endif