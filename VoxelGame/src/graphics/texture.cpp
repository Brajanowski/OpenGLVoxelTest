#include "texture.h"
#include "../stb/stb_image.h"

uint32_t createTexture(uint32_t w, uint32_t h, uint8_t* data, uint32_t internal_format, uint32_t format, uint32_t data_type) {
  bb_assert(w != 0);
  bb_assert(h != 0);

  uint32_t id = 0;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  bb_assert(id != 0);

  // TEMP
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, data_type, data);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return id;
}

uint32_t loadTextureFromFile(const char* filename) {
  int32_t x, y, bpp;
  uint8_t* data = stbi_load(filename, &x, &y, &bpp, 4);

  bb_assert(data != 0);

  uint32_t id = createTexture(x, y, data);

  stbi_image_free(data);
  return id;
}

void removeTexture(uint32_t id) {
  bb_assert(id != 0);

  glDeleteTextures(1, &id);
}

// fbo
FBOData createFBO(uint32_t w, uint32_t h, bool create_depth_buffer) {
  FBOData data;
  data.w = w;
  data.h = h;
  data.textures = 0;
  data.num_textures = 0;
  data.depth_buffer = 0;

  glGenFramebuffers(1, &data.id);
  glBindFramebuffer(GL_FRAMEBUFFER, data.id);

  if (create_depth_buffer) {
    glGenRenderbuffers(1, &data.depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, data.depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, data.depth_buffer);
  }

  return data;
}

void attachTextureToFBO(FBOData* data, uint32_t texture, uint32_t attachment) {
  bb_assert(data != 0);
  bb_assert(data->id != 0);
  bb_assert(texture != 0);

  glBindFramebuffer(GL_FRAMEBUFFER, data->id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);

  // store texture id
  if (data->textures == 0) {
    data->textures = (uint32_t*)malloc(sizeof(uint32_t));
    data->num_textures = 1;
    data->textures[0] = texture;
  } else {
    data->num_textures += 1;
    data->textures = (uint32_t*)realloc(data->textures, data->num_textures * sizeof(uint32_t));
    bb_assert(data->textures != 0);
    data->textures[data->num_textures - 1] = texture;
  }
}

void checkFBOStatus(FBOData* fbo) {
  bb_assert(fbo != 0);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo->id);
  uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    printf("Framebuffer error code: %x", status);
    return;
  }
  

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void removeFBO(FBOData* data, bool remove_textures) {
  bb_assert(data->id != 0);
  glDeleteFramebuffers(1, &data->id);

  if (data->depth_buffer) {
    glDeleteRenderbuffers(1, &data->depth_buffer);
  }

  if (remove_textures) {
    bb_assert(data->num_textures != 0);
    bb_assert(data->textures != 0);

    for (uint32_t i = 0; i < data->num_textures; i++) {
      removeTexture(data->textures[i]);
    }

    free(data->textures);
  }
}
