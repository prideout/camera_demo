-- terrain.vs

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

uniform mat4 modelview;
uniform mat4 inverse_modelview;
uniform mat4 projection;

out vec3 vnormal;
out vec2 vuv;

void main() {
  gl_Position = projection * modelview * position;
  vnormal = (mat3(inverse_modelview) * normal).xyz;
  vuv = uv;
}

-- terrain.fs

in vec2 vuv;
in vec4 vcolor;

out vec4 frag_color;

void main() {
    frag_color = vec4(vuv, 0, 1);
}
