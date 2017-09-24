#pragma once

#include <bb.h>
#include "shader.h"
#include "texture.h"
#include "../world/world.h"
#include "../world/transform.h"

struct DirectionalLight {
  bbVec3 color;
  float intensity;
  bbVec3 direction;
};

struct RendererContext {
  uint32_t w, h;
  FBOData gbuffer;

  // lights
  FBOData light_buffer;

  bbVec3 ambient;
  DirectionalLight sun;

  // fx
  FBOData fx_buffer;

  // generate geometry queue

};

struct RendererInfo {
  bool wireframe;
  bbMat4 camera_projection;
  Transform camera_transform;
  World* world;
};

void initRenderer(RendererContext* ctx);
void drawWorld(RendererContext* ctx, RendererInfo* info);
void deinitRenderer(RendererContext* ctx);
