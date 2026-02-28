layout(location = 0) out vec4 color;


in vec4 v_Colour;
in vec2 v_TexCoord;
 in float v_TexIndex;

uniform float m_TillingFactor;
uniform vec4 u_Color;
uniform sampler2D u_Textures[32];

void main()
{
    vec4 texColor = vec4(1.0);

    // OpenGL requires constant sampler indexing
    if (v_TexIndex ==  0.0)  texColor = texture(u_Textures[0],  v_TexCoord);
else if (v_TexIndex == 1.0)  texColor = texture(u_Textures[1],  v_TexCoord);
else if (v_TexIndex == 2.0)  texColor = texture(u_Textures[2],  v_TexCoord);
else if (v_TexIndex == 3.0)  texColor = texture(u_Textures[3],  v_TexCoord);
else if (v_TexIndex == 4.0)  texColor = texture(u_Textures[4],  v_TexCoord);
else if (v_TexIndex == 5.0)  texColor = texture(u_Textures[5],  v_TexCoord);
else if (v_TexIndex == 6.0)  texColor = texture(u_Textures[6],  v_TexCoord);
else if (v_TexIndex == 7.0)  texColor = texture(u_Textures[7],  v_TexCoord);
else if (v_TexIndex == 8.0)  texColor = texture(u_Textures[8],  v_TexCoord);
else if (v_TexIndex == 9.0)  texColor = texture(u_Textures[9],  v_TexCoord);
else if (v_TexIndex == 11.0) texColor = texture(u_Textures[11], v_TexCoord);
else if (v_TexIndex == 12.0) texColor = texture(u_Textures[12], v_TexCoord);
else if (v_TexIndex == 13.0) texColor = texture(u_Textures[13], v_TexCoord);
else if (v_TexIndex == 10.0) texColor = texture(u_Textures[10], v_TexCoord);
else if (v_TexIndex == 14.0) texColor = texture(u_Textures[14], v_TexCoord);
else if (v_TexIndex == 15.0) texColor = texture(u_Textures[15], v_TexCoord);
else if (v_TexIndex == 16.0) texColor = texture(u_Textures[16], v_TexCoord);
else if (v_TexIndex == 17.0) texColor = texture(u_Textures[17], v_TexCoord);
else if (v_TexIndex == 18.0) texColor = texture(u_Textures[18], v_TexCoord);
else if (v_TexIndex == 19.0) texColor = texture(u_Textures[19], v_TexCoord);
else if (v_TexIndex == 20.0) texColor = texture(u_Textures[20], v_TexCoord);
else if (v_TexIndex == 21.0) texColor = texture(u_Textures[21], v_TexCoord);
else if (v_TexIndex == 22.0) texColor = texture(u_Textures[22], v_TexCoord);
else if (v_TexIndex == 23.0) texColor = texture(u_Textures[23], v_TexCoord);
else if (v_TexIndex == 24.0) texColor = texture(u_Textures[24], v_TexCoord);
else if (v_TexIndex == 25.0) texColor = texture(u_Textures[25], v_TexCoord);
else if (v_TexIndex == 26.0) texColor = texture(u_Textures[26], v_TexCoord);
else if (v_TexIndex == 27.0) texColor = texture(u_Textures[27], v_TexCoord);
else if (v_TexIndex == 28.0) texColor = texture(u_Textures[28], v_TexCoord);
else if (v_TexIndex == 29.0) texColor = texture(u_Textures[29], v_TexCoord);
else if (v_TexIndex == 30.0) texColor = texture(u_Textures[30], v_TexCoord);
else if (v_TexIndex == 31.0) texColor = texture(u_Textures[31], v_TexCoord);

    color = texColor * v_Colour;

    if (color.a <= 0.1)
        discard;
}