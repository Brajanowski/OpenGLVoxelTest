#pragma once

#include <bb.h>
#include <bb_math.h>

struct Shader {
  uint32_t shaders[2];
  uint32_t num_shaders;
  uint32_t program;
};

Shader* loadShaderFromMemory(const char* vertex, const char* fragment);
void removeShader(Shader* shader);
bool compileProgram(uint32_t program);
uint32_t createShader(uint32_t type, const char* source, uint32_t len);

bool setProgramUniform(uint32_t program, const char* name, int32_t value);
bool setProgramUniform(uint32_t program, const char* name, r32 value);
bool setProgramUniform(uint32_t program, const char* name, bbVec2 value);
bool setProgramUniform(uint32_t program, const char* name, bbVec3 value);
bool setProgramUniform(uint32_t program, const char* name, bbVec4 value);
bool setProgramUniform(uint32_t program, const char* name, bbMat4 value);
