#pragma once

static const char vignette_shader_code[] =
  "#version 330\n"
  "out vec3 color;"

  "in vec2 uv0;"
  "uniform sampler2D diffuse;"
  
  "const float RADIUS = 0.75;"
  "const float SOFTNESS = 0.45;"

  "void main() {"
    "vec2 position = (gl_FragCoord.xy / vec2(1600, 900)) - vec2(0.5);" // TODO CHANGE IT TO RES

    "float len = length(position);"
    "float vignette = smoothstep(RADIUS, RADIUS - SOFTNESS, len);"
    "vec3 tex_color = texture(diffuse, uv0).xyz;"
    "color = mix(tex_color.rgb, tex_color.rgb * vignette, 0.5);"
  "}";

static const char ssao_shader_vertex_code[] =
  "#version 330\n"
  "layout (location = 0) in vec3 pos;"
  "layout (location = 1) in vec2 uv;"
  "uniform float aspect_ratio;"
  "uniform float tan_half_fov;"
  "out vec2 uv0;"
  "out vec2 view_ray;"
  "void main () {"
    "gl_Position = vec4(pos, 1.0);"
    "uv0 = uv;"
    "view_ray.x = pos.x * aspect_ratio * tan_half_fov;"
    "view_ray.y = pos.y * tan_half_fov;"
  "}";

static const char ssao_shader_code[] =
  "#version 330\n"
  "out vec3 color;"

  "in vec2 uv0;"
  "in vec2 view_ray;"
  "uniform sampler2D depth_texture;"
  
  "const int KERNEL_SIZE = 64;"
  "const float sample_radius = 1.5;"

  "uniform vec3 kernels[KERNEL_SIZE];"
  "uniform mat4 projection;"

  "float calcViewZ(vec2 coords) {"
    "float depth = texture(depth_texture, coords).x;"
    "float view_z = projection[3][2] / (2 * depth - 1 - projection[2][2]);"
    "return view_z;"
  "}"

  "void main() {"
    "float view_z = calcViewZ(uv0);"
    "float view_x = view_ray.x * view_z;"
    "float view_y = view_ray.y * view_z;"

    "vec3 pos = vec3(view_x, view_y, view_z);"

    "float ao = 0.0;"

    "for (int i = 0; i < KERNEL_SIZE ; i++) {"
      "vec3 sample_pos = pos + kernels[i];"
      "vec4 offset = vec4(sample_pos, 1.0);"
      "offset = projection * offset;"
      "offset.xy /= offset.w;"
      "offset.xy = offset.xy * 0.5 + vec2(0.5);"

      "float sample_depth = calcViewZ(offset.xy);"

      "if (abs(pos.z - sample_depth) < sample_radius) {"
        "ao += step(sample_depth, sample_pos.z);"
      "}"
    "}"

    "ao = 1.0 - ao / 64.0;"

    "color = vec3(pow(ao, 2.0));"//vec3(0, 1, 0);"
  "}";

// lights
static const char deffered_ambient_code[] =
  "#version 330\n"
  "uniform sampler2D diffuse_map;"
  "uniform vec3 ambient;"
  "in vec2 uv0;"
  "out vec3 color;"
  "void main() {"
    "color = texture(diffuse_map, uv0).xyz * ambient;"
  "}";

static const char deffered_directional_code[] =
  "#version 330\n"
  "uniform mat4 view_projection;"
  "uniform sampler2D diffuse_map;"
  "uniform sampler2D normal_map;"
  "uniform sampler2D depth_map;"
  "uniform vec3 light_color;"
  "uniform vec3 light_dir;"
  "uniform float light_intensity;"

  "in vec2 uv0;"
  "out vec3 color;"

  "vec3 calculatePositionFromDepth(vec2 tex_coord, float depth, mat4 vp_inv) {"
    "vec4 clip_space_location;"
    "clip_space_location.xy = tex_coord * 2.0 - 1.0;"
    "clip_space_location.z = depth * 2.0 - 1.0;"
    "clip_space_location.w = 1.0;"
    "vec4 homogenous_location = vp_inv * clip_space_location;"
    "return homogenous_location.xyz / homogenous_location.w;"
  "}"

  "void main() {"
    "vec3 normal = normalize(texture(normal_map, uv0).xyz);"
    "float diffuse_factor = max(0.0, dot(normal, -light_dir));"
    "color = texture(diffuse_map, uv0).xyz * (light_color * light_intensity * diffuse_factor);"
  "}";

static const char* gbuffer_vert = 
  "#version 330\n"
  "layout (location = 0) in vec3 pos;"
  "layout (location = 1) in vec3 normal;"
  "layout (location = 2) in vec3 color;"
  "out vec3 normal0;"
  "out vec3 color0;"
  "uniform mat4 mvp;"
  "uniform mat4 model;"
  "void main() {"
  "  gl_Position = mvp * vec4(pos, 1.0);"
  "  normal0 = normalize(vec4(model * vec4(normal, 0.0)).xyz);"
  "  color0 = color;"
  "}";

static const char* gbuffer_frag =
  "#version 330\n"
  "layout (location = 0) out vec3 rt0;"
  "layout (location = 1) out vec3 rt1;"
  "in vec3 normal0;"
  "in vec3 color0;"
  "void main() {"
  "  rt0 = color0;"
  "  rt1 = normalize(normal0);"
  "}";

static const char* fullscreen_quad_vert =
  "#version 330\n"
  "layout (location = 0) in vec3 pos;"
  "layout (location = 1) in vec2 uv;"
  "out vec2 uv0;"
  "void main () {"
  "  gl_Position = vec4(pos, 1.0);"
  "  uv0 = uv;"
  "}";

static const char* frame_frag =
  "#version 330\n"
  "out vec3 color;"
  "in vec2 uv0;"
  "uniform sampler2D diffuse;"

  "void main() {"
  "  color = texture(diffuse, uv0).xyz;"
  "}";

static const char* standard_shader_vert =
  "#version 330\n"

  "layout (location = 0) in vec3 pos;"
  "layout (location = 1) in vec3 normal;"
  "layout (location = 2) in vec3 color;"

  "out vec3 normal0;"
  "out vec3 color0;"

  "uniform mat4 mvp;"
  "uniform mat4 model;"

  "void main() {"
    "gl_Position = mvp * vec4(pos, 1.0);"
    "normal0 = normalize(vec4(model * vec4(normal, 0.0)).xyz);"
    "color0 = color;"
  "};";

static const char* standard_shader_frag =
  "#version 330\n"

  "out vec3 color;"
  "in vec3 normal0;"
  "in vec3 color0;"

  "void main() {"
    "color = color0;"
  "}";

static const char* flat_shader_vert =
  "#version 330\n"

  "layout (location = 0) in vec3 pos;"

  "uniform mat4 mvp;"

  "void main() {"
    "gl_Position = mvp * vec4(pos, 1.0);"
  "};";

static const char* flat_shader_frag =
  "#version 330\n"
  "out vec3 col;"
  "uniform vec3 color = vec3(1, 0, 1);"
  "void main() {"
    "col = color;"
  "}";
