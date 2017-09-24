#pragma once

#include <bb.h>
#include <bb_math.h>

#include "transform.h"

#define CHUNK_SIZE_X    16
#define CHUNK_SIZE_Y    128
#define CHUNK_SIZE_Z    16
#define VOXEL_SIZE      1.0f //0.0625f
#define VOXEL_SIZE_HALF 0.5f //0.03125f

struct Voxel {
  uint8_t r, g, b;
  bool active;
};

#define POSITION_BUFFER   0
#define NORMAL_BUFFER     1
#define COLOR_BUFFER      2
#define INDICES_BUFFER    3
#define MAX_BUFFERS       4

#define CHUNK_NOT_READY     0
#define CHUNK_PROCESSING    1
#define CHUNK_READY         2

struct Chunk {
  uint8_t status;

  Voxel voxels[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];

  uint32_t vertex_buffers[MAX_BUFFERS];
  uint32_t vao;
  uint32_t draw_count;
};

struct ChunkGeometry {
  bbVec3* pos;
  bbVec3* normal;
  bbVec3* color;
  uint32_t vertex_number;
  uint32_t* indices;
  uint32_t indices_number;
};

struct World;
ChunkGeometry* generateGeometry(Chunk* chunk, World* world, int32_t begin_x, int32_t begin_y);
void uploadGeometry(Chunk* chunk, ChunkGeometry* geometry);
void removeGeometry(Chunk* chunk);

