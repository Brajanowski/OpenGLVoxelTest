#include "shader.h"

#include <GL/glew.h>

// TODO: expose to lower level api
static bool getShaderErrors(const char error[1024], uint32_t shader, uint32_t flag, bool is_program);

Shader* loadShaderFromMemory(const char* vertex, const char* fragment) {
  Shader* shader = (Shader*)malloc(sizeof(Shader));
  bb_assert(shader != 0);

  shader->program = glCreateProgram();
  bb_assert(shader->program != 0);

  shader->shaders[0] = createShader(GL_VERTEX_SHADER, vertex, (uint32_t)strlen(vertex));
  shader->shaders[1] = createShader(GL_FRAGMENT_SHADER, fragment, (uint32_t)strlen(fragment));

  glAttachShader(shader->program, shader->shaders[0]);
  glAttachShader(shader->program, shader->shaders[1]);

  compileProgram(shader->program);

  return shader;
}

uint32_t createShader(uint32_t type, const char* source, uint32_t len) {
  uint32_t shader = glCreateShader(type);
  bb_assert(shader != 0);

  const GLchar* p[1];
  p[0] = source;
  GLint lengths[1];
  lengths[0] = (GLint)len;

  glShaderSource(shader, 1, p, lengths);
  glCompileShader(shader);

  int32_t success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    char info_log[1024];

    glGetShaderInfoLog(shader, 1024, 0, info_log);

    printf("compiling shader error: %s\n", info_log);
    bb_assert(true);
    return 0;
  }

  return shader;
}

bool compileProgram(uint32_t program) {
  char error[1024];

  glLinkProgram(program);
  memset((void*)error, '\0', sizeof(1024));
  if (getShaderErrors(error, program, GL_LINK_STATUS, true)) {
    printf("linking shader program error: %s\n", error);
    return false;
  }

  glValidateProgram(program);
  memset((void*)error, '\0', sizeof(1024));
  if (getShaderErrors(error, program, GL_VALIDATE_STATUS, true)) {
    printf("invalid shader program: %s\n", error);
    return false;
  }

  return true;
}

void removeShader(Shader* shader) {
  bb_assert(shader != 0);

  glDetachShader(shader->program, shader->shaders[0]);
  glDeleteShader(shader->shaders[0]);

  glDetachShader(shader->program, shader->shaders[1]);
  glDeleteShader(shader->shaders[1]);

  glDeleteProgram(shader->program);

  free(shader->shaders);
}

bool setProgramUniform(uint32_t program, const char* name, int32_t value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUniform1i(location, value);
    return true;
  }

  return false;
}

bool setProgramUniform(uint32_t program, const char* name, r32 value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUniform1f(location, value);
    return true;
  }

  return false;
}


bool setProgramUniform(uint32_t program, const char* name, bbVec2 value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUniform2f(location, value.x, value.y);
    return true;
  }

  return false;
}

bool setProgramUniform(uint32_t program, const char* name, bbVec3 value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUniform3f(location, value.x, value.y, value.z);
    return true;
  }

  return false;
}

bool setProgramUniform(uint32_t program, const char* name, bbVec4 value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUniform4f(location, value.x, value.y, value.z, value.w);
    return true;
  }

  return false;
}

bool setProgramUniform(uint32_t program, const char* name, bbMat4 value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUniformMatrix4fv(location, 1, GL_TRUE, &(value[0][0]));
    return true;
  }

  return false;
}

static bool getShaderErrors(const char error[1024], uint32_t shader, uint32_t flag, bool is_program) {
  GLint success = 0;

  if (is_program) {
    glGetProgramiv(shader, flag, &success);
  } else {
    glGetShaderiv(shader, flag, &success);
  }

  if (!success) {
    if (is_program) {
      glGetProgramInfoLog(shader, sizeof(error), 0, (GLchar*)error);
    } else {
      glGetShaderInfoLog(shader, sizeof(error), 0, (GLchar*)error);
    }

    return true;
  }

  return false;
}
