attribute vec3 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoord;
uniform vec4 u_LightPos;
uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_NormalMatrix;
uniform vec4 u_Color;
varying vec4 v_Color;
varying vec2 v_TexCoord;

void main(void)
{
  gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1);
  v_Color = u_Color * vec4(0.2, 0.2, 0.2, 1) + u_Color * max(dot(normalize(u_LightPos.xyz), normalize(vec3(u_NormalMatrix * vec4(a_Normal, 1)))), 0.0);
  v_TexCoord = a_TexCoord;
}
