#version 420

layout(binding = 0) uniform u {
  vec4 u_LightPos;
  mat4 u_ModelViewProjectionMatrix;
  mat4 u_NormalMatrix;
  vec4 u_Color;
  int u_TextureFlag;
};
layout(binding = 1) uniform sampler2D u_Texture;
layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;

void main()
{
  if (u_TextureFlag == 1) {
    vec4 t = texture(u_Texture, v_TexCoord);
    if (t.a == 0.0)
      FragColor = v_Color;
    else
      FragColor = t;
  }
  else
    FragColor = v_Color;
}
