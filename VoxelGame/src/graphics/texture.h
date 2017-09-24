#pragma once

#include <bb.h>
#include <GL/glew.h>

uint32_t createTexture(uint32_t w, uint32_t h, uint8_t* data, 
                       uint32_t internal_format = GL_RGBA, uint32_t format = GL_RGBA,
                       uint32_t data_type = GL_UNSIGNED_BYTE);

uint32_t loadTextureFromFile(const char* filename);

void removeTexture(uint32_t id);

// fbo
struct FBOData {
  uint32_t id;
  uint32_t w, h;

  uint32_t* textures;
  uint32_t num_textures;

  // This variable is optional, use
  uint32_t depth_buffer;
};

FBOData createFBO(uint32_t w, uint32_t h, bool create_depth_buffer = false);
void attachTextureToFBO(FBOData* data, uint32_t texture, uint32_t attachment);
void checkFBOStatus(FBOData* fbo);
void removeFBO(FBOData* data, bool remove_texture = true);

