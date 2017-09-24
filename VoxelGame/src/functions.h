#pragma once

struct World;
struct MeshWorkerContext;
struct Transform;

void initMeshBuildThreads(MeshWorkerContext* ctx);
void closeMeshBuildThreads();
void meshWorkerHandler(void* param); // takes pointer to MeshWorkerContext struct
void mainMeshWorker(World* world, Transform* camera_transform);
