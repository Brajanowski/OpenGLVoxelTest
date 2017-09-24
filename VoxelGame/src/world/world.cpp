#include "world.h"
#include <GL/glew.h>
#include <bb_math.h>

#include "../stb/stb_perlin.h"
#include "../graphics/renderer.h"

void initWorld(World* world) {
  bb_assert(world != 0);
  memset(world->chunks, 0, sizeof(world->chunks));
}

void unloadWorld(World* world) {
  bb_assert(world != 0);

  for (uint32_t y = 0; y < WORLD_SIZE; y++) {
    for (uint32_t x = 0; x < WORLD_SIZE; x++) {
      if (world->chunks[x][y] != 0) {
        if (world->chunks[x][y]->vao != 0) {
          removeGeometry(world->chunks[x][y]);
        }

        free(world->chunks[x][y]);
        world->chunks[x][y] = 0;
      }
    }
  }
}

Chunk* getChunk(World* world, int32_t chunk_x, int32_t chunk_y) {
  uint32_t cx = bb_abs(chunk_x);
  uint32_t cy = bb_abs(chunk_y);
  
  Chunk* chunk = world->chunks[cx][cy];

  if (chunk == 0) {
    chunk = (Chunk*)malloc(sizeof(Chunk));
    chunk->vao = 0;
    chunk->draw_count = 0;
    memset(chunk, 0, sizeof(Chunk));
    world->chunks[cx][cy] = chunk;

    if (cx == 256 && cy == 256) {
      for (uint32_t z = 0; z < CHUNK_SIZE_Z; z++) { 
        for (uint32_t x = 0; x < CHUNK_SIZE_X; x++) {
          for (uint32_t y = 0; y < CHUNK_SIZE_Y / 2; y++) {
            if (y == 0) {
              chunk->voxels[x][y][z].r = 128;
              chunk->voxels[x][y][z].g = 72;
              chunk->voxels[x][y][z].b = 128;
              chunk->voxels[x][y][z].active = true;
            }

            if ((x == 0 && z == 0) ||
                (x == CHUNK_SIZE_X - 1 && z == CHUNK_SIZE_Z - 1) ||
                (x == 0 && z == CHUNK_SIZE_Z - 1) ||
                (x == CHUNK_SIZE_X - 1 && z == 0)) {
              if (y == (uint32_t)(CHUNK_SIZE_Y / 2) - 1) {
                chunk->voxels[x][y][z].r = 32;
                chunk->voxels[x][y][z].g = 32;
                chunk->voxels[x][y][z].b = 32;
              } else {
                chunk->voxels[x][y][z].r = 128;
                chunk->voxels[x][y][z].g = 72;
                chunk->voxels[x][y][z].b = 128;
              }

              chunk->voxels[x][y][z].active = true;
            }
          }
        }
      }
    } else {
      for (uint32_t z = 0; z < CHUNK_SIZE_Z; z++) { 
        for (uint32_t x = 0; x < CHUNK_SIZE_X; x++) {
          r32 height = 0.12f * ((stb_perlin_noise3(((chunk_x * CHUNK_SIZE_X) + x) * 0.03f, ((chunk_y * CHUNK_SIZE_Z) + z) * 0.03f, 0.0f) + 1.0f) / 2.0f) * (CHUNK_SIZE_Y - 1);

          if (height < 0)
            continue;

          for (uint32_t y = 0; y < (uint32_t)height; y++) {
            if (y == (uint32_t)height - 1) {
              chunk->voxels[x][y][z].r = 48;
              chunk->voxels[x][y][z].g = 200;//height * 255;
              chunk->voxels[x][y][z].b = 32;
            } else {
              chunk->voxels[x][y][z].r = bb_max(y, 24);
              chunk->voxels[x][y][z].g = bb_max(y, 24);
              chunk->voxels[x][y][z].b = bb_max(y, 24);
            }

            chunk->voxels[x][y][z].active = true;
          }
        }
      }
    }
  }

  return chunk;
}

Voxel* getVoxel(World* world, int32_t x, int32_t y, int32_t z) {
  int32_t chunk_x = (int32_t)(x / CHUNK_SIZE_X);
  int32_t chunk_y = (int32_t)(z / CHUNK_SIZE_Z);

  Chunk* chunk = getChunk(world, chunk_x, chunk_y);

  uint32_t voxel_x = x - (chunk_x * CHUNK_SIZE_X);
  uint32_t voxel_y = y;
  uint32_t voxel_z = z - (chunk_y * CHUNK_SIZE_Z);

  bb_assert(voxel_x < CHUNK_SIZE_X);
  bb_assert(voxel_y < CHUNK_SIZE_Y);
  bb_assert(voxel_z < CHUNK_SIZE_Z);
  
  return &chunk->voxels[voxel_x][voxel_y][voxel_z];
}

void worldTick(World* world, r32 dt) {
}
