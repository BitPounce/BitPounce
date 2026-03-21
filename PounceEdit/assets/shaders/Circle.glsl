#type vertex

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Colour;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;

layout (location = 0) out float v_Fade;
layout (location = 1) out vec3 v_LocalPosition;
layout (location = 2) out vec4 v_Colour;
layout (location = 3) out float v_Thickness;
layout (location = 4) flat out int v_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	v_LocalPosition = a_LocalPosition;
	v_Colour = a_Colour;
	v_Thickness = a_Thickness;
	v_Fade = a_Fade;

	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
layout(location = 0) out vec4 colour;
layout(location = 1) out int colour2;

layout (location = 0) in float v_Fade;
layout (location = 1) in vec3 v_LocalPosition;
layout (location = 2) in vec4 v_Colour;
layout (location = 3) in float v_Thickness;
layout (location = 4) flat in int v_EntityID;

void main()
{
    float dis = 1.0 - length(v_LocalPosition);
    float circle = smoothstep(0.0, v_Fade, dis);
    circle *= smoothstep(v_Thickness + v_Fade, v_Thickness, dis);

    if (circle == 0.0)
		discard;

    // Set output color
    colour = v_Colour;
	colour.a *= circle;

	colour2 = v_EntityID;

}