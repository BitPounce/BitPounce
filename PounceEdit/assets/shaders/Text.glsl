#type vertex

// Vertex Shader
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_EntityID; // optional editor info

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
flat out int v_EntityID;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_EntityID = a_EntityID;

    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment

in vec4 v_Color;
in vec2 v_TexCoord;
flat in int v_EntityID;

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

uniform sampler2D u_FontAtlas;

// ====== Controls ======
float u_MtsdfBlend = 0.2;     // 0 = MSDF, 1 = SDF
float u_Softness   = 0.0;     // edge softness

// Outline
vec4  u_OutlineColor = vec4(0.0, 0.0, 0.0, 1.0);
float u_OutlineWidth = 0.0;

// Shadow
vec2  u_ShadowOffset = vec2(0.0);
vec4  u_ShadowColor  = vec4(0.0, 0.0, 0.0, 0.5);

vec2 sqr(vec2 x) { return x*x; }

float screenPxRange() {
    const float pxRange = 3.0;
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_FontAtlas, 0));
    vec2 screenTexSize = inversesqrt(sqr(dFdx(v_TexCoord)) + sqr(dFdy(v_TexCoord)));
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float sampleDistance(vec2 uv)
{
    vec4 tex = texture(u_FontAtlas, uv);

    float msdf = median(tex.r, tex.g, tex.b);
    float sdf  = tex.a;

    return mix(msdf, sdf, u_MtsdfBlend);
}

void main()
{



    // ===== Main distance =====
    float sd = sampleDistance(v_TexCoord);

    // Proper anti-aliasing
    float w = fwidth(sd);
    float alpha = smoothstep(0.5 - w - u_Softness,
                             0.5 + w + u_Softness,
                             sd);

    if (alpha <= 0.0)
        discard;

    // ===== Outline =====
    float outline = 0.0;
    if (u_OutlineWidth > 0.0)
    {
        outline = smoothstep(
            0.5 - u_OutlineWidth - w,
            0.5 - u_OutlineWidth + w,
            sd
        );
    }

    // ===== Shadow =====
    float shadowAlpha = 0.0;
    if (length(u_ShadowOffset) > 0.0)
    {
        float shadowSD = sampleDistance(v_TexCoord + u_ShadowOffset);
        shadowAlpha = smoothstep(0.5 - w, 0.5 + w, shadowSD);
    }

    // ===== Combine layers =====
    vec3 baseColor = v_Color.rgb;
    vec3 finalColor = baseColor;

    // Shadow behind
    finalColor = mix(u_ShadowColor.rgb, finalColor, alpha);
    float finalAlpha = max(alpha, shadowAlpha * u_ShadowColor.a);

    // Outline
    if (u_OutlineWidth > 0.0)
    {
        finalColor = mix(u_OutlineColor.rgb, finalColor, alpha);
        finalAlpha = max(finalAlpha, outline);
    }

    // Apply vertex alpha
    finalAlpha *= v_Color.a;

    if (finalAlpha <= 0.0)
        discard;

    // ===== Gamma correction (important!) =====
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    color = vec4(finalColor, finalAlpha);
    color2 = v_EntityID;
}