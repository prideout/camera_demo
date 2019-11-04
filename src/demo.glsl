-- terrain.vs

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;

uniform mat4 modelview;
uniform mat4 inverse_modelview;
uniform mat4 projection;

out vec3 vnormal;
out vec2 vuv;

void main() {
  gl_Position = projection * modelview * vec4(position.xy, 0.0, 1);
  vnormal = (mat3(inverse_modelview) * normal).xyz;
  vuv = position.xy * vec2(1.0, 3.0);
}

-- terrain.fs

in vec2 vuv;
in vec4 vcolor;

uniform sampler2D terrain;

out vec4 frag_color;

void main() {
    frag_color = texture(terrain, vuv);
}
