// Basic Texture Shader

#type vertex

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Colour;
out vec2 v_TexCoord;

void main()
{
	v_Colour = a_Colour;
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment

layout(location = 0) out vec4 color;


in vec4 v_Colour;
in vec2 v_TexCoord;

uniform float m_TillingFactor;
uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
	color = v_Colour;

	if(color.a <= 0.1)
	{
		discard;
	}
}