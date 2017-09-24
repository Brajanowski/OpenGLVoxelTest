#pragma once

#include <bb_thread.h>
#include <vector>

#define RENDER_DISTANCE 16
#define MAX_WORKERS 4

struct LoadedChunk {
  Chunk* chunk;
  uint32_t x, y;
};

struct MeshWorkerContext {
  World* world;
};
