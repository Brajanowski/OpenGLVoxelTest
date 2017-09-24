#pragma once

#include <bb_math.h>

struct Transform {
  bbVec3 position;
  bbQuaternion rotation;
};

static bbMat4 calculateModelMatrix(Transform* transform) {
  return (bb_translate(transform->position) * bb_rotate(transform->rotation));
}

static void rotateTransform(Transform* transform, const bbQuaternion& q) {
  transform->rotation = bb_normalize(q * transform->rotation);
}

static void rotateTransform(Transform* transform, r32 angle, const bbVec3& axis) {
  rotateTransform(transform, bbQuaternion(angle, axis));
}


