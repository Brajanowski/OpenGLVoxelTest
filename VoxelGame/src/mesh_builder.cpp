#include "functions.h"
#include "world/world.h"
#include "data.h"

#include <bb.h>
#include <bb_thread.h>
#include <vector>

bbThread* mesh_workers[MAX_WORKERS];

bbMutex task_mutex;
bbMutex main_task_mutex;

#define JOB_GENERATE_GEOMETRY 1
#define JOB_GENERATE_TERRAIN  2

struct Task {
  uint8_t type;
  int32_t x, y;

  Chunk* chunk;
};

std::vector<Task*> task_queue;

void addToQueue(Task* task) {
  bb_lock(&task_mutex);
  task_queue.push_back(task);
  bb_unlock(&task_mutex);
}

Task* getTask() {
  Task* task = 0;

  bb_lock(&task_mutex);
  if (task_queue.size() > 0) {
    task = task_queue.back();
    task_queue.pop_back();
  }
  bb_unlock(&task_mutex);
  return task;
}


#define MAIN_TASK_UPLOAD_GEOMETRY 1
#define MAIN_TASK_REMOVE_GEOMETRY 2

struct MainTask {
  uint8_t type;
  Chunk* chunk;
  ChunkGeometry* geometry;
  uint32_t x, y;
};

std::vector<MainTask> main_task_queue;

extern bbMutex world_chunks_mutex;
extern bbMutex loaded_chunks_mutex;
extern std::vector<LoadedChunk> loaded_chunks;

void initMeshBuildThreads(MeshWorkerContext* ctx) {
  for (uint32_t i = 0; i < MAX_WORKERS; i++)
    mesh_workers[i] = bb_createThread(meshWorkerHandler, ctx);

  task_mutex = bb_createMutex();
  main_task_mutex = bb_createMutex();
}

void closeMeshBuildThreads() {
  for (uint32_t i = 0; i < MAX_WORKERS; i++)
    bb_closeThread(mesh_workers[i]);

  bb_deleteMutex(&task_mutex);
  bb_deleteMutex(&main_task_mutex);
}

void meshWorkerHandler(void* data) {
  MeshWorkerContext* ctx = (MeshWorkerContext*)data;

  for (;;) {
    //if (exit_signal)
    //  return;

    Task* task = getTask();

    if (task != 0) {
      if (task->type == JOB_GENERATE_GEOMETRY) {
        ChunkGeometry* geometry = generateGeometry(task->chunk, ctx->world, task->x * CHUNK_SIZE_X, task->y * CHUNK_SIZE_Z);

        bb_lock(&main_task_mutex);

        MainTask main_task;
        main_task.chunk = task->chunk;
        main_task.geometry = geometry;
        main_task.type = MAIN_TASK_UPLOAD_GEOMETRY;
        main_task.x = task->x;
        main_task.y = task->y;

        main_task_queue.push_back(main_task);

        bb_unlock(&main_task_mutex);
      }

      free(task);
    }

    Sleep(1);
  }
}

void mainMeshWorker(World* world, Transform* camera_transform) {
  // check visible chunks
  bbVec3 cam_pos = camera_transform->position;

  int32_t camera_chunk_x = (int32_t)(cam_pos.x / CHUNK_SIZE_X);
  int32_t camera_chunk_y = (int32_t)(cam_pos.z / CHUNK_SIZE_Z);

  int32_t begin_chunk_x = camera_chunk_x - RENDER_DISTANCE;
  int32_t begin_chunk_y = camera_chunk_y - RENDER_DISTANCE;

  int32_t end_chunk_x = camera_chunk_x + RENDER_DISTANCE;
  int32_t end_chunk_y = camera_chunk_y + RENDER_DISTANCE;

  for (int32_t chunk_y = begin_chunk_y; chunk_y < end_chunk_y; chunk_y++) {
    for (int32_t chunk_x = begin_chunk_x; chunk_x < end_chunk_x; chunk_x++) {
      bb_lock(&world_chunks_mutex);
      if (world->chunks[chunk_x][chunk_y] == 0 || world->chunks[chunk_x][chunk_y]->status == CHUNK_NOT_READY) {
        //printf("creating chunk at: %dx%d\n", chunk_x, chunk_y);
        Chunk* chunk = getChunk(world, chunk_x, chunk_y);
        chunk->status = CHUNK_PROCESSING;

        Task* task = (Task*)malloc(sizeof(Task));
        task->chunk = chunk;
        task->type = JOB_GENERATE_GEOMETRY;
        task->x = chunk_x;
        task->y = chunk_y;

        addToQueue(task);
      } else if (world->chunks[chunk_x][chunk_y]->vao == 0){
        //printf("vao == 0\n");
      }
      bb_unlock(&world_chunks_mutex);
    }
  }

  // remove chunks that are not in our render distance
  bb_lock(&loaded_chunks_mutex);

  for (auto i = loaded_chunks.begin(); i != loaded_chunks.end();) {
    if (i->x < begin_chunk_x || i->y < begin_chunk_y ||
        i->x > end_chunk_x || i->y > end_chunk_y) {

      bb_lock(&main_task_mutex);

      MainTask main_task;
      main_task.chunk = i->chunk;
      main_task.geometry = 0;
      main_task.type = MAIN_TASK_REMOVE_GEOMETRY;
      main_task.x = i->x;
      main_task.y = i->y;

      main_task_queue.push_back(main_task);

      bb_unlock(&main_task_mutex);

      i = loaded_chunks.erase(i);
    } else {
      ++i;
    }
  }

  bb_unlock(&loaded_chunks_mutex);

  // handle main tasks queue
  if (bb_tryLock(&main_task_mutex)) {
    for (uint32_t i = 0; i < main_task_queue.size(); i++) {
      if (main_task_queue[i].type == MAIN_TASK_UPLOAD_GEOMETRY) {

        bb_lock(&world_chunks_mutex);
        uploadGeometry(main_task_queue[i].chunk, main_task_queue[i].geometry);
        main_task_queue[i].chunk->status = CHUNK_READY;
        bb_unlock(&world_chunks_mutex);

        // add to loaded chunks list
        LoadedChunk lchunk;
        lchunk.chunk = main_task_queue[i].chunk;
        lchunk.x = main_task_queue[i].x;
        lchunk.y = main_task_queue[i].y;

        bb_lock(&loaded_chunks_mutex);

        loaded_chunks.push_back(lchunk);

        bb_unlock(&loaded_chunks_mutex);

        // free memory
        free(main_task_queue[i].geometry->pos);
        free(main_task_queue[i].geometry->normal);
        free(main_task_queue[i].geometry->color);
        free(main_task_queue[i].geometry->indices);
        free(main_task_queue[i].geometry);
      } else if (main_task_queue[i].type == MAIN_TASK_REMOVE_GEOMETRY) {
        removeGeometry(main_task_queue[i].chunk);

        bb_lock(&world_chunks_mutex);
        
        free(world->chunks[main_task_queue[i].x][main_task_queue[i].y]);
        world->chunks[main_task_queue[i].x][main_task_queue[i].y] = 0;
        
        bb_unlock(&world_chunks_mutex);
      }
    }
    main_task_queue.clear();

    bb_unlock(&main_task_mutex);
  }
}
