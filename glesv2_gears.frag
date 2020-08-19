precision mediump float;
uniform int u_TextureEnable;
uniform sampler2D u_Texture;
varying vec4 v_Color;
varying vec2 v_TexCoord;

void main()
{
  if (u_TextureEnable == 1) {
    vec4 t = texture2D(u_Texture, v_TexCoord);
    if (t.a == 0.0)
      gl_FragColor = v_Color;
    else
      gl_FragColor = t;
  }
  else
    gl_FragColor = v_Color;
}
