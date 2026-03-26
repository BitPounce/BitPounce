#type vertex

in vec3 a_Position;
in vec4 a_Colour;
in int a_EntityID;

uniform mat4 u_ViewProjection;

out vec4 v_Colour;
flat out int v_EntityID;

void main()
{
	v_Colour = a_Colour;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int o_EntityID;

in vec4 v_Colour;
flat in int v_EntityID;


void main()
{
	o_Colour = v_Colour;
	o_EntityID = v_EntityID;
}