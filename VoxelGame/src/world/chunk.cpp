#include "chunk.h"
#include "world.h"
#include <bb_math.h>
#include <bb_thread.h>

#define STB_PERLIN_IMPLEMENTATION
#include "../stb/stb_perlin.h"

#include <vector> // TODO: REMOVE THIS 

#include <GL/glew.h>

#define BUFFER_START_SIZE 128

ChunkGeometry* generateGeometry(Chunk* chunk, World* world, int32_t begin_x, int32_t begin_y) {
  bb_assert(chunk != 0);
  //bb_assert(world != 0);

  // TODO: replace std::vector with custom container
  std::vector<bbVec3> vertices;
  std::vector<bbVec3> normals;
  std::vector<bbVec3> color;
  std::vector<uint32_t> indices;

  int32_t begin_vertex = -1;

  for (uint32_t z = 0; z < CHUNK_SIZE_Z; z++) {
    for (uint32_t y = 0; y < CHUNK_SIZE_Y; y++) {
      for (uint32_t x = 0; x < CHUNK_SIZE_X; x++) {
        if (chunk->voxels[x][y][z].active) {
          // basic optimization
          bool xneg = false;
          bool xpos = false;
          bool yneg = false;
          bool ypos = false;
          bool zneg = false;
          bool zpos = false;

          if (world != 0) {
            // xpos test
            {
              Voxel* voxel = getVoxel(world, begin_x + x + 1, y, begin_y + z);

              if (voxel != 0) {
                if (voxel->active) 
                  xpos = true;
                else 
                  xpos = false;
              }
            }

            // xneg test
            {
              Voxel* voxel = getVoxel(world, begin_x + x - 1, y, begin_y + z);

              if (voxel != 0) {
                if (voxel->active) 
                  xneg = true; 
                else
                  xneg = false;
              }
            }

            if (y == CHUNK_SIZE_Y - 1) 
              ypos = false; 
            else if (chunk->voxels[x][y + 1][z].active) 
              ypos = true;

            if (y == 0)
              yneg = true;
            else if (chunk->voxels[x][y - 1][z].active)
              yneg = true;

            // zpos test
            {
              Voxel* voxel = getVoxel(world, begin_x + x, y, begin_y + z + 1);

              if (voxel != 0) {
                if (voxel->active) 
                  zpos = true;
                else 
                  zpos = false;
              }
            }

            // zneg test
            {
              Voxel* voxel = getVoxel(world, begin_x + x, y, begin_y + z - 1);

              if (voxel != 0) {
                if (voxel->active) 
                  zneg = true;
                else 
                  zneg = false;
              }
            }
          } else {
            if (x == 0)
              xneg = false;
            else if (chunk->voxels[x - 1][y][z].active)
              xneg = true;

            if (x == CHUNK_SIZE_X - 1)
              xpos = false;
            else if (chunk->voxels[x + 1][y][z].active)
              xpos = true;

            if (y == 0)
              yneg = true;
            else if (chunk->voxels[x][y - 1][z].active)
              yneg = true;

            if (y == CHUNK_SIZE_Y - 1)
              ypos = true;
            else if (chunk->voxels[x][y + 1][z].active)
              ypos = true;

            if (z == 0)
              zneg = false;
            else if (chunk->voxels[x][y][z - 1].active)
              zneg = true;

            if (z == CHUNK_SIZE_Z - 1)
              zpos = false;
            else if (chunk->voxels[x][y][z + 1].active)
              zpos = true;
          }

          // generate vertices
          if (!zneg) {
            if (begin_vertex >= 0)
              begin_vertex += 4;
            else if (begin_vertex == -1)
              begin_vertex = 0;

            // front
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));

            normals.push_back(bbVec3(0, 0, -1));
            normals.push_back(bbVec3(0, 0, -1));
            normals.push_back(bbVec3(0, 0, -1));
            normals.push_back(bbVec3(0, 0, -1));
            
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));

            indices.push_back(begin_vertex + 0);
            indices.push_back(begin_vertex + 1);
            indices.push_back(begin_vertex + 2);

            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 3);
            indices.push_back(begin_vertex + 0);
          }

          // back
          if (!zpos) {
            if (begin_vertex >= 0)
              begin_vertex += 4;
            else if (begin_vertex == -1)
              begin_vertex = 0;

            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));

            normals.push_back(bbVec3(0, 0, 1));
            normals.push_back(bbVec3(0, 0, 1));
            normals.push_back(bbVec3(0, 0, 1));
            normals.push_back(bbVec3(0, 0, 1));
            
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));

            indices.push_back(begin_vertex + 0);
            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 1);

            indices.push_back(begin_vertex + 3);
            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 0);
          }

          // top
          if (!ypos) {
            if (begin_vertex >= 0)
              begin_vertex += 4;
            else if (begin_vertex == -1)
              begin_vertex = 0;

            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));

            normals.push_back(bbVec3(0, 1, 0));
            normals.push_back(bbVec3(0, 1, 0));
            normals.push_back(bbVec3(0, 1, 0));
            normals.push_back(bbVec3(0, 1, 0));
            
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));

            indices.push_back(begin_vertex + 0);
            indices.push_back(begin_vertex + 1);
            indices.push_back(begin_vertex + 2);

            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 3);
            indices.push_back(begin_vertex + 0);
          }

          // bottom
          if (!yneg) {
            if (begin_vertex >= 0)
              begin_vertex += 4;
            else if (begin_vertex == -1)
              begin_vertex = 0;

            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));

            normals.push_back(bbVec3(0, -1, 0));
            normals.push_back(bbVec3(0, -1, 0));
            normals.push_back(bbVec3(0, -1, 0));
            normals.push_back(bbVec3(0, -1, 0));
            
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));

            indices.push_back(begin_vertex + 0);
            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 1);

            indices.push_back(begin_vertex + 3);
            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 0);
          }

          // left
          if (!xneg) {
            if (begin_vertex >= 0)
              begin_vertex += 4;
            else if (begin_vertex == -1)
              begin_vertex = 0;

            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) - VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));

            normals.push_back(bbVec3(-1, 0, 0));
            normals.push_back(bbVec3(-1, 0, 0));
            normals.push_back(bbVec3(-1, 0, 0));
            normals.push_back(bbVec3(-1, 0, 0));
            
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));

            indices.push_back(begin_vertex + 0);
            indices.push_back(begin_vertex + 1);
            indices.push_back(begin_vertex + 2);

            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 3);
            indices.push_back(begin_vertex + 0);
          }

          // right
          if (!xpos) {
            if (begin_vertex >= 0)
              begin_vertex += 4;
            else if (begin_vertex == -1)
              begin_vertex = 0;

            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) - VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) + VOXEL_SIZE_HALF));
            vertices.push_back(bbVec3((x * VOXEL_SIZE) + VOXEL_SIZE_HALF, (y * VOXEL_SIZE) + VOXEL_SIZE_HALF, (z * VOXEL_SIZE) - VOXEL_SIZE_HALF));

            normals.push_back(bbVec3(1, 0, 0));
            normals.push_back(bbVec3(1, 0, 0));
            normals.push_back(bbVec3(1, 0, 0));
            normals.push_back(bbVec3(1, 0, 0));
            
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));
            color.push_back(bbVec3(chunk->voxels[x][y][z].r / 255.0f, chunk->voxels[x][y][z].g / 255.0f, chunk->voxels[x][y][z].b / 255.0f));

            indices.push_back(begin_vertex + 0);
            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 1);

            indices.push_back(begin_vertex + 3);
            indices.push_back(begin_vertex + 2);
            indices.push_back(begin_vertex + 0);
          }
        }
      }
    }
  }

  chunk->draw_count = indices.size();

  //uploadGeometry(chunk);

  ChunkGeometry* geometry = (ChunkGeometry*)malloc(sizeof(ChunkGeometry));
  size_t vertices_number = vertices.size();

  geometry->pos = (bbVec3*)malloc(vertices_number * sizeof(bbVec3));
  geometry->normal = (bbVec3*)malloc(vertices_number * sizeof(bbVec3));
  geometry->color = (bbVec3*)malloc(vertices_number* sizeof(bbVec3));
  geometry->vertex_number = vertices_number;
  geometry->indices = (uint32_t*)malloc(indices.size() * sizeof(uint32_t));
  geometry->indices_number = indices.size();

  memcpy(geometry->pos, &vertices[0], geometry->vertex_number * sizeof(bbVec3));
  memcpy(geometry->normal, &normals[0], geometry->vertex_number * sizeof(bbVec3));
  memcpy(geometry->color, &color[0], geometry->vertex_number * sizeof(bbVec3));
  memcpy(geometry->indices, &indices[0], geometry->indices_number * sizeof(uint32_t));
 
  return geometry;
}

void uploadGeometry(Chunk* chunk, ChunkGeometry* geometry) {
  bb_assert(chunk != 0);
  bb_assert(geometry != 0);
  bb_assert(geometry->pos != 0);
  bb_assert(geometry->normal != 0);
  bb_assert(geometry->color != 0);
  bb_assert(geometry->indices != 0);
  bb_assert(geometry->vertex_number != 0);
  bb_assert(geometry->indices_number != 0);

  if (chunk->vao == 0 && chunk->draw_count > 0) {
    glGenVertexArrays(1, &chunk->vao);
    glBindVertexArray(chunk->vao);

    glGenBuffers(MAX_BUFFERS, chunk->vertex_buffers);

    // positions
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vertex_buffers[POSITION_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, geometry->vertex_number * sizeof(bbVec3), geometry->pos, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // normals
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vertex_buffers[NORMAL_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, geometry->vertex_number * sizeof(bbVec3), geometry->normal, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // colors
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vertex_buffers[COLOR_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, geometry->vertex_number * sizeof(bbVec3), geometry->color, GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // indicies
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->vertex_buffers[INDICES_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry->indices_number * sizeof(uint32_t), geometry->indices, GL_STATIC_DRAW);
  } else {
    printf("Error while trying to upload geometry, vao=%d, draw_count=%d\n", chunk->vao, chunk->draw_count);
  }
}

void removeGeometry(Chunk* chunk) {
  bb_assert(chunk != 0);

  glDeleteBuffers(MAX_BUFFERS, chunk->vertex_buffers);
  glDeleteVertexArrays(1, &chunk->vao);

  chunk->vao = 0;
  chunk->draw_count = 0;
}
