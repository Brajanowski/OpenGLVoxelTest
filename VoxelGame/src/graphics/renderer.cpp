#include "renderer.h"

#include <vector>
#include <GL/glew.h>

#include "../data.h"
#include "shaders.h"
#include "frustum.h"

static const int SSAO_KERNEL_SIZE = 64;

// shaders
extern Shader* gbuffer_shader;
extern Shader* display_frame_shader;
extern Shader* deffered_ambient;
extern Shader* deffered_directional;
extern Shader* vignette_shader;
extern Shader* ssao_shader;

extern uint32_t quad_vao;

bbMutex loaded_chunks_mutex;
std::vector<LoadedChunk> loaded_chunks;

void initRenderer(RendererContext* ctx) {
  // init gbuffer
  ctx->gbuffer = createFBO(ctx->w, ctx->h);
  uint32_t albedo = createTexture(ctx->w, ctx->h, 0, GL_RGB, GL_RGB);
  uint32_t normal = createTexture(ctx->w, ctx->h, 0, GL_RGB, GL_RGB);
  uint32_t depth = createTexture(ctx->w, ctx->h, 0, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

  attachTextureToFBO(&ctx->gbuffer, albedo, GL_COLOR_ATTACHMENT0);
  attachTextureToFBO(&ctx->gbuffer, normal, GL_COLOR_ATTACHMENT1);
  attachTextureToFBO(&ctx->gbuffer, depth, GL_DEPTH_STENCIL_ATTACHMENT);

  checkFBOStatus(&ctx->gbuffer);

  // init frame buffer
  ctx->fx_buffer = createFBO(ctx->w, ctx->h, false);
  uint32_t frame_texture = createTexture(ctx->w, ctx->h, 0, GL_RGB, GL_RGB);
  attachTextureToFBO(&ctx->fx_buffer, frame_texture, GL_COLOR_ATTACHMENT0);

  checkFBOStatus(&ctx->fx_buffer);

  // init light buffer
  ctx->light_buffer = createFBO(ctx->w, ctx->h, false);
  uint32_t light_texture = createTexture(ctx->w, ctx->h, 0, GL_RGB, GL_RGB);
  attachTextureToFBO(&ctx->light_buffer, light_texture, GL_COLOR_ATTACHMENT0);

  checkFBOStatus(&ctx->light_buffer);

  // enable culling faces
  glFrontFace(GL_CW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  // set up depth functions
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_CLAMP);

  // generate kernels for ssao
  glUseProgram(ssao_shader->program);
  bbVec3 kernel[SSAO_KERNEL_SIZE];
    
  for (uint32_t i = 0 ; i < SSAO_KERNEL_SIZE ; i++ ) {
    r32 scale = (r32)i / (float)(SSAO_KERNEL_SIZE);        
    bbVec3 v;
    v.x = 2.0f * (r32)rand() / RAND_MAX - 1.0f;
    v.y = 2.0f * (r32)rand() / RAND_MAX - 1.0f;
    v.z = 2.0f * (r32)rand() / RAND_MAX - 1.0f;

    v *= (0.1f + 0.9f * scale * scale);
        
    kernel[i] = v;
  }

  uint32_t kernels_location = glGetUniformLocation(ssao_shader->program, "kernels");
  if (kernels_location != -1) {
    glUniform3fv(kernels_location, SSAO_KERNEL_SIZE, (const GLfloat*)&kernel[0]); 
  }

  // init lighting
  ctx->ambient = bbVec3(0.45f, 0.45f, 0.45f);
  ctx->sun.color = bbVec3(1, 1, 1);
  ctx->sun.intensity = 0.95f;
  ctx->sun.direction = bbVec3(-0.5, -0.5, 0);

  loaded_chunks_mutex = bb_createMutex();
}

void deinitRenderer(RendererContext* ctx) {
  removeFBO(&ctx->gbuffer, true);
  removeFBO(&ctx->fx_buffer, true);
  removeFBO(&ctx->light_buffer, true);

  bb_deleteMutex(&loaded_chunks_mutex);
}

void drawWorld(RendererContext* ctx, RendererInfo* info) {
  // fill gbuffer
  glBindFramebuffer(GL_FRAMEBUFFER, ctx->gbuffer.id);

  uint32_t draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, draw_buffers);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  glClearColor(0, 0, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (info->wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  bbVec3 cam_pos = info->camera_transform.position; 

  int32_t camera_chunk_x = (int32_t)(cam_pos.x / CHUNK_SIZE_X);
  int32_t camera_chunk_y = (int32_t)(cam_pos.z / CHUNK_SIZE_Z);

  bbMat4 view_projection = info->camera_projection * bb_rotate(bb_conjugate(info->camera_transform.rotation)) * bb_translate(info->camera_transform.position * -1.0f);

  Frustum frustum = computeFrustum(view_projection);

  bb_lock(&loaded_chunks_mutex);
  for (size_t i = 0, size = loaded_chunks.size(); i < size; i++) {
    Chunk* chunk = loaded_chunks[i].chunk;

    r32 pos_x = loaded_chunks[i].x * CHUNK_SIZE_X * VOXEL_SIZE;
    r32 pos_z = loaded_chunks[i].y * CHUNK_SIZE_Z * VOXEL_SIZE;

    if (!isPointInFrustum(&frustum, bbVec3(pos_x, 0, pos_z)))
      continue;

    glUseProgram(gbuffer_shader->program);

    bbMat4 model = (bb_translate(bbVec3(pos_x, 0, pos_z)) * bb_rotate(bbQuaternion(0, 0, 0, 1)) * bb_scale(bbVec3(1, 1, 1)));
    bbMat4 mvp = view_projection * model;
  
    setProgramUniform(gbuffer_shader->program, "mvp", mvp);
    setProgramUniform(gbuffer_shader->program, "model", model);

    glBindVertexArray(chunk->vao);
    glDrawElements(GL_TRIANGLES, chunk->draw_count, GL_UNSIGNED_INT, 0);
  }
  bb_unlock(&loaded_chunks_mutex);

  if (info->wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // lights
  //bbMat4 view_projection = info->camera_projection * bb_rotate(bb_conjugate(info->camera_transform.rotation)) * bb_translate(info->camera_transform.position * -1.0f);

  glBindFramebuffer(GL_FRAMEBUFFER, ctx->light_buffer.id);
  glViewport(0, 0, ctx->w, ctx->h);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
    
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  // ambient
  glUseProgram(deffered_ambient->program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ctx->gbuffer.textures[0]);
  setProgramUniform(deffered_ambient->program, "diffuse_map", 0);

  setProgramUniform(deffered_ambient->program, "ambient", ctx->ambient);
  
  glBindVertexArray(quad_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  // sun
  glUseProgram(deffered_directional->program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ctx->gbuffer.textures[0]);
  setProgramUniform(deffered_directional->program, "diffuse_map", 0);
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, ctx->gbuffer.textures[1]);
  setProgramUniform(deffered_directional->program, "normal_map", 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, ctx->gbuffer.textures[2]);
  setProgramUniform(deffered_directional->program, "depth_map", 2);

  setProgramUniform(deffered_directional->program, "view_projection", view_projection);

  setProgramUniform(deffered_directional->program, "light_color", ctx->sun.color);
  setProgramUniform(deffered_directional->program, "light_intensity", ctx->sun.intensity);
  setProgramUniform(deffered_directional->program, "light_dir", ctx->sun.direction);
  
  glBindVertexArray(quad_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  
  glDisable(GL_BLEND);

  // postfx
  // TODO: COMBINE STUFF TO FRAME BUFFER BEFORE STARTING WORKING ON FXS
  glBindFramebuffer(GL_FRAMEBUFFER, ctx->fx_buffer.id);
  glViewport(0, 0, ctx->w, ctx->h);
  
  // vignette
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ctx->light_buffer.textures[0]);

  glUseProgram(vignette_shader->program);

  glBindVertexArray(quad_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  // ssao
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, ctx->gbuffer.textures[3]); // BIND DEPTH TEXTURE

  //glUseProgram(ctx->ssao_shader->program);
  //setProgramUniform(ctx->ssao_shader->program, "projection", info->camera_projection);
  //setProgramUniform(ctx->ssao_shader->program, "aspect_ratio", ((r32)ctx->w / (r32)ctx->h));
  //setProgramUniform(ctx->ssao_shader->program, "tan_half_fov", tanf(bb_toRadiansReal32(70.0f / 2.0f)));

  //glBindVertexArray(ctx->quad_vao);
  //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  // frame
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx->w, ctx->h);

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ctx->fx_buffer.textures[0]);

  glUseProgram(display_frame_shader->program);
  glBindVertexArray(quad_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
