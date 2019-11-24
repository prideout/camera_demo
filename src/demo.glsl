-- terrain.vs

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;

uniform mat4 modelview;
uniform mat4 inverse_modelview;
uniform mat4 projection;
uniform vec2 map_extent;
uniform vec2 map_center;

out vec3 vnormal;
out vec2 vuv;
out float vdepth;
out vec3 vpos;

void main() {
  gl_Position = projection * modelview * position;
  vpos = position.xyz;
  vnormal = (mat3(inverse_modelview) * normal).xyz;
  const float aspect = 3200.0 / 1335.0;
  vuv = position.xy * vec2(1.0, aspect);
  vdepth = position.z;
}

-- ocean.vs

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;

uniform mat4 modelview;
uniform mat4 inverse_modelview;
uniform mat4 projection;
uniform vec2 map_extent;
uniform vec2 map_center;

out vec3 vnormal;
out vec2 vuv;
out float vdepth;
out vec3 vpos;

void main() {

  vec4 corners[4];
  corners[0] = vec4( map_center[0] - map_extent[0]/2.0, map_center[1] - map_extent[1]/2.0, 0.0, 1.0 );
  corners[1] = vec4( map_center[0] + map_extent[0]/2.0, map_center[1] - map_extent[1]/2.0, 0.0, 1.0 );
  corners[2] = vec4( map_center[0] + map_extent[0]/2.0, map_center[1] + map_extent[1]/2.0, 0.0, 1.0 );
  corners[3] = vec4( map_center[0] - map_extent[0]/2.0, map_center[1] + map_extent[1]/2.0, 0.0, 1.0 );

  vec4 p;
  if (gl_VertexID == 0) { p = corners[0]; }
  if (gl_VertexID == 1) { p = corners[1]; }
  if (gl_VertexID == 2) { p = corners[2]; }
  if (gl_VertexID == 3) { p = corners[2]; }
  if (gl_VertexID == 4) { p = corners[3]; }
  if (gl_VertexID == 5) { p = corners[0]; }

  gl_Position = projection * modelview * p;
  vpos = position.xyz;
  vnormal = (mat3(inverse_modelview) * normal).xyz;
  const float aspect = 3200.0 / 1335.0;
  vuv = p.xy * vec2(1.0, aspect);
  vdepth = p.z;
}

-- terrain.fs

in vec2 vuv;
in vec4 vcolor;
in float vdepth;
in vec3 vpos;

uniform mat4 modelview;
uniform mat4 inverse_modelview;
uniform mat4 projection;
uniform vec2 map_extent;
uniform vec2 map_center;

uniform sampler2D terrain;

out vec4 frag_color;

void main() {
    frag_color = texture(terrain, vuv);
    frag_color.a = 1.0;
}
