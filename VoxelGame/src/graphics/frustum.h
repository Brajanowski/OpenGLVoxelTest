#pragma once

#include <bb_math.h>

#define FRUSTUM_NEAR    0
#define FRUSTUM_FAR     1
#define FRUSTUM_LEFT    2
#define FRUSTUM_RIGHT   3
#define FRUSTUM_TOP     4
#define FRUSTUM_BOTTOM  5

struct Plane {
  bbVec3 normal;
  r32 offset;
};

static Plane normalize(Plane plane) {
  r32 l = bb_length(plane.normal);
  
  Plane res;
  res.normal = plane.normal / l;
  res.offset = plane.offset / l;

  return res;
}

// six planes that represents frustum
struct Frustum {
  Plane planes[6];
};

// compute frustum planes from camera view projection matrix
static Frustum computeFrustum(bbMat4 m) {
  Frustum frustum;

  frustum.planes[FRUSTUM_RIGHT].normal.x = m[0][3] - m[0][0];
  frustum.planes[FRUSTUM_RIGHT].normal.y = m[1][3] - m[1][0];
  frustum.planes[FRUSTUM_RIGHT].normal.z = m[2][3] - m[2][0];
  frustum.planes[FRUSTUM_RIGHT].offset   = m[3][3] - m[3][0];

  frustum.planes[FRUSTUM_LEFT].normal.x = m[0][3] + m[0][0];
  frustum.planes[FRUSTUM_LEFT].normal.y = m[1][3] + m[1][0];
  frustum.planes[FRUSTUM_LEFT].normal.z = m[2][3] + m[2][0];
  frustum.planes[FRUSTUM_LEFT].offset   = m[3][3] + m[3][0];

  frustum.planes[FRUSTUM_BOTTOM].normal.x = m[0][3] + m[0][1];
  frustum.planes[FRUSTUM_BOTTOM].normal.y = m[1][3] + m[1][1];
  frustum.planes[FRUSTUM_BOTTOM].normal.z = m[2][3] + m[2][1];
  frustum.planes[FRUSTUM_BOTTOM].offset   = m[3][3] + m[3][1];

  frustum.planes[FRUSTUM_TOP].normal.x = m[0][3] - m[0][1];
  frustum.planes[FRUSTUM_TOP].normal.y = m[1][3] - m[1][1];
  frustum.planes[FRUSTUM_TOP].normal.z = m[2][3] - m[2][1];
  frustum.planes[FRUSTUM_TOP].offset   = m[3][3] - m[3][1];

  frustum.planes[FRUSTUM_FAR].normal.x = m[0][3] - m[0][2];
  frustum.planes[FRUSTUM_FAR].normal.y = m[1][3] - m[1][2];
  frustum.planes[FRUSTUM_FAR].normal.z = m[2][3] - m[2][2];
  frustum.planes[FRUSTUM_FAR].offset   = m[3][3] - m[3][2];

  frustum.planes[FRUSTUM_NEAR].normal.x = m[0][3] + m[0][2];
  frustum.planes[FRUSTUM_NEAR].normal.y = m[1][3] + m[1][2];
  frustum.planes[FRUSTUM_NEAR].normal.z = m[2][3] + m[2][2];
  frustum.planes[FRUSTUM_NEAR].offset   = m[3][3] + m[3][2];

  for (uint32_t i = 0; i < 6; i++) {
    frustum.planes[i] = normalize(frustum.planes[i]);
  }

  return frustum;
}

static bool isPointInFrustum(Frustum* frustum, const bbVec3& point) {
  for (uint32_t i = 0; i < 6; i++) {
    if (bb_dot(point, frustum->planes[i].normal) + frustum->planes[i].offset + 1.0f <= 0)
      return false;
  }
  return true;
}

static bool isAABBInFrustum(Frustum* frustum, const bbVec3& min, const bbVec3& max) {

  return true;
}
