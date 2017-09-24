#include <bb.h>
#include <GL/glew.h>
#include <ctime>

#include "../graphics/shader.h"
#include "../world/chunk.h"
#include "../platform/input.h"
#include "../platform/window.h"

// temp
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

// imgui
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl_gl3.h"

static Chunk chunk;
static bbMat4 camera_projection;
static Transform camera_transform;

static bool draw_chunk_borders = true;

static void chunkEditorCamera(Transform* transform, r32 dt, Input* input);

extern Shader* standard_shader;
extern Shader* flat_shader;
extern Window* window;

extern uint32_t cube_vao;

static bbVec3 voxel_pos;

enum Mode {
	ERASE = 0,
	PLACE = 1
};

static Mode current_mode = PLACE;

void initChunkEditor() {
  memset(&chunk, 0, sizeof(Chunk));

  for (uint32_t z = 0; z < CHUNK_SIZE_Z; z++) {
    for (uint32_t y = 0; y < CHUNK_SIZE_Y; y++) {
      for (uint32_t x = 0; x < CHUNK_SIZE_X; x++) {
        if (y == CHUNK_SIZE_Y / 2) {
          chunk.voxels[x][y][z].active = true;
          chunk.voxels[x][y][z].r = 24;
          chunk.voxels[x][y][z].g = 180;
          chunk.voxels[x][y][z].b = 32;
        } else if (y < CHUNK_SIZE_Y / 2) {
          chunk.voxels[x][y][z].active = true;
          chunk.voxels[x][y][z].r = 36;
          chunk.voxels[x][y][z].g = 44;
          chunk.voxels[x][y][z].b = 31;
        } else {
          chunk.voxels[x][y][z].active = false;
        }
      }
    }
  }

  ChunkGeometry* geometry = generateGeometry(&chunk, 0, 0, 0);
  uploadGeometry(&chunk, geometry);

  free(geometry->pos);
  free(geometry->normal);
  free(geometry->color);
  free(geometry->indices);
  free(geometry);

  camera_transform.position = bbVec3(CHUNK_SIZE_X / 2, CHUNK_SIZE_Y / 2, -10);
  camera_projection = bb_perspective(bb_toRadiansReal32(70.0f), ((r32)WINDOW_WIDTH / (r32)WINDOW_HEIGHT), 0.1f, 1000.0f);
}

void closeChunkEditor() {
  removeGeometry(&chunk);
}

void rayTestAABB(bbVec3 ray_pos, bbVec3 ray_dir, bbVec3 aabb_min, bbVec3 aabb_max, float distance) {
	
}

void updateChunkEditor(r32 dt, Input* input) {
  chunkEditorCamera(&camera_transform, dt, input);
  
  //glClearColor(0.52f, 0.8f, 0.92f, 1);
	glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(standard_shader->program);

  bbMat4 view_projection = camera_projection * bb_rotate(bb_conjugate(camera_transform.rotation)) * bb_translate(camera_transform.position * -1.0f);
  bbMat4 model = (bb_translate(bbVec3(0, 0, 0)) * bb_rotate(bbQuaternion(0, 0, 0, 1)) * bb_scale(bbVec3(1, 1, 1)));
  bbMat4 mvp = view_projection * model;
  
  setProgramUniform(standard_shader->program, "mvp", mvp);
  setProgramUniform(standard_shader->program, "model", model);

  glBindVertexArray(chunk.vao);
  glDrawElements(GL_TRIANGLES, chunk.draw_count, GL_UNSIGNED_INT, 0);

	if (current_mode == PLACE) {
		// update
		if (input->buttons_down[MOUSE_LEFT_BUTTON]) {
			rayTestAABB(camera_transform.position, bb_forward(camera_transform.rotation), bbVec3(0, 0, 0), bbVec3(16, 128, 16), 1000.0f);
		}

		// render
		glUseProgram(flat_shader->program);

    model = (bb_translate(voxel_pos) * bb_rotate(bbQuaternion(0, 0, 0, 1)) * bb_scale(bbVec3(1, 1, 1)));
    mvp = view_projection * model;
  
    setProgramUniform(flat_shader->program, "mvp", mvp);
		setProgramUniform(flat_shader->program, "color", bbVec3(1, 0, 1));

    // draw
    glBindVertexArray(cube_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}

  /*if (draw_chunk_borders) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);

		glUseProgram(flat_shader->program);

    model = (bb_translate(bbVec3(0, CHUNK_SIZE_Y / 2.0f, 0)) * bb_rotate(bbQuaternion(0, 0, 0, 1)) * bb_scale(bbVec3(CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z)));
    mvp = view_projection * model;
  
    setProgramUniform(flat_shader->program, "mvp", mvp);
		setProgramUniform(flat_shader->program, "color", bbVec3(1, 0, 0));
    //setProgramUniform(standard_shader->program, "model", model);

    // draw
    glBindVertexArray(cube_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }*/

  // gui stuff
  ImGui::Begin("Chunk editor");

    ImGui::Button("Save");
    ImGui::Button("Load");

    //ImGui::Checkbox("Draw chunk borders", &draw_chunk_borders);

  ImGui::End();
}

static void chunkEditorCamera(Transform* transform, r32 dt, Input* input) {
  static bool is_mouse_locked = false;
  static bbVec2 mouse_locked_pos;
    
  if (input->buttons_up[MOUSE_RIGHT_BUTTON]) {
    is_mouse_locked = false;
  }

  if (input->buttons_down[MOUSE_RIGHT_BUTTON]) {
    is_mouse_locked = true;
    mouse_locked_pos = bbVec2((r32)input->mouse_x, (r32)input->mouse_y);
  }

  if (is_mouse_locked) {
    bbVec2 delta = bbVec2((r32)input->mouse_x, (r32)input->mouse_y) - mouse_locked_pos;
    bool rotate_x = delta.y != 0;
    bool rotate_y = delta.x != 0;

    if (rotate_y) {
      rotateTransform(transform, delta.x * 0.2f, bbVec3(0, 1, 0));
    }

    if (rotate_x) {
      rotateTransform(transform, delta.y * 0.2f, bb_right(transform->rotation));
    }

    if (rotate_x || rotate_y) {
      SDL_WarpMouseInWindow(window->sdl_window, (int32_t)mouse_locked_pos.x, (int32_t)mouse_locked_pos.y);
    }

    r32 speed = 2.0f;

    if (input->keys[KEY_SPACE]) {
      speed = 20.0f;
    }

    if (input->keys[KEY_W]) {
      transform->position += bb_forward(transform->rotation) * (speed * dt);
    } else if (input->keys[KEY_S]) {
      transform->position += bb_back(transform->rotation) * (speed * dt);
    }

    if (input->keys[KEY_A]) {
      transform->position += bb_left(transform->rotation) * (speed * dt);
    } else if (input->keys[KEY_D]) {
      transform->position += bb_right(transform->rotation) * (speed * dt);
    }
  }
}