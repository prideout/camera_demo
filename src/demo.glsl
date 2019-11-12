-- terrain.vs

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;

uniform mat4 modelview;
uniform mat4 inverse_modelview;
uniform mat4 projection;

out vec3 vnormal;
out vec2 vuv;
out float vdepth;

void main() {
  gl_Position = projection * modelview * position;
  vnormal = (mat3(inverse_modelview) * normal).xyz;
  vuv = position.xy * vec2(1.0, 3.0);
  vdepth = position.z;
}

-- terrain.fs

in vec2 vuv;
in vec4 vcolor;
in float vdepth;

uniform sampler2D terrain;

out vec4 frag_color;

void main() {
    frag_color = texture(terrain, vuv);
    frag_color.a = 1.0;
}
