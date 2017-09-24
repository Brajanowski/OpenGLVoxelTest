#include "shader.h"
#include "shaders.h"

Shader* gbuffer_shader = 0;
Shader* display_frame_shader = 0;
Shader* deffered_ambient = 0;
Shader* deffered_directional = 0;
Shader* vignette_shader = 0;
Shader* ssao_shader = 0;
Shader* standard_shader = 0;
Shader* flat_shader = 0;

void loadShaders() {
  gbuffer_shader = loadShaderFromMemory(gbuffer_vert, gbuffer_frag);
  display_frame_shader = loadShaderFromMemory(fullscreen_quad_vert, frame_frag);
  deffered_ambient = loadShaderFromMemory(fullscreen_quad_vert, deffered_ambient_code);
  deffered_directional = loadShaderFromMemory(fullscreen_quad_vert, deffered_directional_code);

  vignette_shader = loadShaderFromMemory(fullscreen_quad_vert, vignette_shader_code);
  ssao_shader = loadShaderFromMemory(ssao_shader_vertex_code, ssao_shader_code);

  standard_shader = loadShaderFromMemory(standard_shader_vert, standard_shader_frag);
  flat_shader = loadShaderFromMemory(flat_shader_vert, flat_shader_frag);
}

void unloadShaders() {
  removeShader(gbuffer_shader);
  removeShader(display_frame_shader);
  removeShader(deffered_ambient);
  removeShader(deffered_directional);
  removeShader(vignette_shader);
  removeShader(ssao_shader);
}
