#version 420

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(binding = 0) uniform u {
  vec4 u_LightPos;
  mat4 u_ModelViewProjectionMatrix;
  mat4 u_NormalMatrix;
  vec4 u_Color;
  int u_TextureFlag;
};
layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;

void main()
{
  gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1);
  v_Color = u_Color * vec4(0.2, 0.2, 0.2, 1) + u_Color * max(dot(normalize(u_LightPos.xyz), normalize(vec3(u_NormalMatrix * vec4(a_Normal, 1)))), 0.0);
  v_TexCoord = a_TexCoord;
}
