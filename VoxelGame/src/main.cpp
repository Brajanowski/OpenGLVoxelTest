#include <bb.h>
#include <bb_thread.h>
#include <bb_memory.h>

#include "platform/window.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/renderer.h"
#include "world/chunk.h"

#include "functions.h"
#include "data.h"

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"

#include "world/transform.h"

#include <GL/glew.h>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

static World world;
static bool wireframed = false;

static RendererContext renderer_ctx;
Window* window = 0;

// mode
#define MODE_MAIN_MENU      0x00
#define MODE_SINGLEPLAYER   0x10
#define MODE_MULTIPLAYER    0x11
#define MODE_CHUNK_EDITOR   0x20
#define MODE_VOXEL_EDITOR   0x21

static uint8_t game_mode = MODE_SINGLEPLAYER;

extern void initChunkEditor();
extern void updateChunkEditor(r32 dt, Input* input);
extern void closeChunkEditor();

// profiler stuff
static uint32_t renderer_time = 0;
static uint32_t update_time = 0;
static uint32_t ui_time = 0;

// DEBUG CAMERA
static bbMat4 camera_projection;
static Transform camera_transform;

bbMutex world_chunks_mutex;

void cameraEditor(Transform* transform, Input* input, r32 dt) {
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

static void init() {
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
  printf("GLSL version supported %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  renderer_ctx.w = WINDOW_WIDTH;
  renderer_ctx.h = WINDOW_HEIGHT;

  initRenderer(&renderer_ctx);
  
  // world
  initWorld(&world);
  world.seed = 123;
  world.time = 0;
  
  // INIT CAMERA DEBUG:
  camera_projection = bb_perspective(bb_toRadiansReal32(70.0f), ((r32)WINDOW_WIDTH / (r32)WINDOW_HEIGHT), 0.1f, 1000.0f);
  //camera_transform.position = bbVec3(0, 15, 0);
  camera_transform.position = bbVec3((uint32_t)(WORLD_SIZE / 2) * CHUNK_SIZE_X, 15, (uint32_t)(WORLD_SIZE / 2) * CHUNK_SIZE_X);
  camera_transform.rotation = bbQuaternion(0.1f, 0.85f, -0.19f, 0.45f);

  // mutex
  world_chunks_mutex = bb_createMutex();

  // chunk editor
  initChunkEditor();
}

static void exitApp() {
  deinitRenderer(&renderer_ctx);
  unloadWorld(&world);

  bb_deleteMutex(&world_chunks_mutex);

  closeChunkEditor();
}

static void gameLoop(r32 dt) {
  if (game_mode == MODE_SINGLEPLAYER) {
    update_time = SDL_GetTicks();
  
    mainMeshWorker(&world, &camera_transform);

    cameraEditor(&camera_transform, &window->input, dt);
    worldTick(&world, dt);

    update_time = SDL_GetTicks() - update_time;

    // draw
    renderer_time = SDL_GetTicks();
  
    RendererInfo info;
    info.wireframe = wireframed;
    info.camera_projection = camera_projection;
    info.camera_transform = camera_transform;
    info.world = &world;
    drawWorld(&renderer_ctx, &info);

    renderer_time = SDL_GetTicks() - renderer_time; 
  } else if (game_mode == MODE_CHUNK_EDITOR) {
    updateChunkEditor(dt, &window->input);
  }

  // editor gui goes here
  ImGui::Begin("Game editor");
    ImGui::Text("FPS: %d", (uint32_t)ImGui::GetIO().Framerate);

    ImGui::Text("Block: %d, %d, %d", (int32_t)camera_transform.position.x, (int32_t)camera_transform.position.y, (int32_t)camera_transform.position.z);
    ImGui::Text("Chunk: %d, %d", (int32_t)((int32_t)camera_transform.position.x / CHUNK_SIZE_X), (int32_t)((int32_t)camera_transform.position.z / CHUNK_SIZE_Z));

    if (ImGui::CollapsingHeader("Profiler")) {
      ImGui::Text("Renderer: %dms", renderer_time);
      ImGui::Text("Update: %dms", update_time);
      ImGui::Text("UI: %dms", ui_time);
    }

    if (ImGui::CollapsingHeader("Camera")) {
      ImGui::InputFloat3("cam pos", &camera_transform.position.x);
      ImGui::InputFloat4("cam rot", &camera_transform.rotation.x);
    }

    if (ImGui::CollapsingHeader("Renderer")) {
      ImGui::Checkbox("wireframe", &wireframed);
    }
    
  ImGui::End();
}

extern void loadShaders();
extern void unloadShaders();

extern void loadPrimitives();
extern void unloadPrimitives();

#undef main
int main(int argc, const char** argv) {
  bb_memInit();

  window = openWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Voxel game", false);

	loadPrimitives();
  loadShaders();

  ImGui_ImplSdlGL3_Init(window->sdl_window);

  init();

  MeshWorkerContext ctx;
  ctx.world = &world;

  initMeshBuildThreads(&ctx);

  r32 delta_time = 0.0f;
  uint32_t end_time = 0;

  for (;;) {
    catchEvents(window);
    if (window->is_close_requested)
      break;

    // time
    uint32_t start_time = SDL_GetTicks();
    delta_time = (start_time - end_time) / 1000.0f;
    end_time = start_time;

    // frame
    ImGui_ImplSdlGL3_NewFrame(window->sdl_window);

    gameLoop(delta_time);

    ui_time = SDL_GetTicks();
    ImGui::Render();
    ui_time = SDL_GetTicks() - ui_time;

    swapBuffers(window);
  }
  exitApp();

  ImGui_ImplSdlGL3_Shutdown();
  closeWindow(window);

  unloadShaders();
	unloadPrimitives();

  closeMeshBuildThreads();

  bb_freeMem();
  return 0;
}
