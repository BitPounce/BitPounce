#type vertex

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Colour;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;

out float v_Fade;
out vec3 v_LocalPosition;
out vec4 v_Colour;
out float v_Thickness;
flat out int v_EntityID;

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
layout(location = 1) out int colour2; // integer output

in float v_Fade;
in vec3 v_LocalPosition;
in vec4 v_Colour;
in float v_Thickness;
flat in int v_EntityID;

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