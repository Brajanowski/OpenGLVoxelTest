#include <bb.h>
#include <GL/glew.h>

uint32_t quad_vbo[3];
uint32_t quad_vao;

static const r32 quad_vertex_data[] = {
  -1.0f, -1.0f, 0.0f,
  -1.0f, 1.0f, 0.0f,
  1.0f, 1.0f, 0.0f,
  1.0f, -1.0f, 0.0f
};

static const r32 quad_uv_data[] = {
  0.0f, 0.0f,
  0.0f, 1.0f,
  1.0f, 1.0f,
  1.0f, 0.0f
};

static const uint32_t quad_indices_data[] = {
  0, 1, 2,
  2, 3, 0
};

uint32_t cube_vbo[3];
uint32_t cube_vao;

static const r32 cube_vertex_data[] = {
   0.5f, -0.5f, -0.5f,
   0.5f, -0.5f,	 0.5f,
  -0.5f, -0.5f,	 0.5f,
  -0.5f, -0.5f, -0.5f,
   0.5f,	0.5f,	-0.5f,
   0.5f,	0.5f,	 0.5f,
  -0.5f,	0.5f,	 0.5f,
  -0.5f,	0.5f, -0.5f
};

static const r32 cube_normal_data[] = {
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, -1.0f
};

static const uint32_t cube_indices_data[] = {
  0, 1, 2,
  7, 5, 4,
  4, 1, 0,
  5, 2, 1,
  2, 7, 3,
  0, 7, 4,
  1, 2, 3,
  7, 6, 5,
  4, 5, 1,
  5, 6, 2,
  2, 6, 7,
  0, 3, 7
};

void loadPrimitives() {
  // fullscreen quad
  glGenVertexArrays(1, &quad_vao);
  glBindVertexArray(quad_vao);

  glGenBuffers(3, quad_vbo);

  // positions
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_data), quad_vertex_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // uvs
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_uv_data), quad_uv_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
  
  // indicies
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices_data), quad_indices_data, GL_STATIC_DRAW);

  // cube
  glGenVertexArrays(1, &cube_vao);
  glBindVertexArray(cube_vao);

  glGenBuffers(3, cube_vbo);

  // positions
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_data), cube_vertex_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // normals
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normal_data), cube_normal_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  
  // indicies
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices_data), cube_indices_data, GL_STATIC_DRAW);
}

void unloadPrimitives() {
  glDeleteBuffers(3, quad_vbo);
  glDeleteVertexArrays(1, &quad_vao);

  glDeleteBuffers(3, cube_vbo);
  glDeleteVertexArrays(1, &cube_vao);
}
