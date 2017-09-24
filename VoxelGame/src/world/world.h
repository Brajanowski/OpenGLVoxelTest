#pragma once

#include <bb.h>
#include "chunk.h"

#define WORLD_SIZE 512 // world is squared

struct World {
  char path[256];
  uint64_t seed;

  // chunks
  Chunk* chunks[WORLD_SIZE][WORLD_SIZE];

  // some world data like time
  r32 time;
};

void initWorld(World* world);
void unloadWorld(World* world);

Chunk* getChunk(World* world, int32_t chunk_x, int32_t chunk_y);
Voxel* getVoxel(World* world, int32_t x, int32_t y, int32_t z);
void worldTick(World* world, r32 dt);

