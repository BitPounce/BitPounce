#type vertex

// Vertex Shader
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int  a_TexID;           // <-- new
layout(location = 3) in int a_EntityID; // optional editor info

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
flat out int v_TexIndex;      // <-- pass to fragment as flat
flat out int v_EntityID;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexID;
    v_EntityID = a_EntityID;

    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment

in vec4 v_Color;
in vec2 v_TexCoord;
flat in int v_TexIndex;      // <-- receives the texture slot index
flat in int v_EntityID;

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

uniform sampler2D u_Textures[32];

// ====== Controls ======
float u_MtsdfBlend = 0.2;
float u_Softness   = 0.0;

vec4  u_OutlineColor = vec4(0.0, 0.0, 0.0, 1.0);
float u_OutlineWidth = 0.0;

vec2  u_ShadowOffset = vec2(0.0);
vec4  u_ShadowColor  = vec4(0.0, 0.0, 0.0, 0.5);

vec2 sqr(vec2 x) { return x*x; }

float screenPxRange() {
    const float pxRange = 3.0;
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_Textures[0], 0));
    vec2 screenTexSize = inversesqrt(sqr(dFdx(v_TexCoord)) + sqr(dFdy(v_TexCoord)));
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float sampleDistance(vec2 uv, int texIndex)
{
    // The big if-else chain – compiler will flatten to constant-index texture fetches
    vec4 tex;
         if (texIndex == 0)  tex = texture(u_Textures[0], uv);
    else if (texIndex == 1)  tex = texture(u_Textures[1], uv);
    else if (texIndex == 2)  tex = texture(u_Textures[2], uv);
    else if (texIndex == 3)  tex = texture(u_Textures[3], uv);
    else if (texIndex == 4)  tex = texture(u_Textures[4], uv);
    else if (texIndex == 5)  tex = texture(u_Textures[5], uv);
    else if (texIndex == 6)  tex = texture(u_Textures[6], uv);
    else if (texIndex == 7)  tex = texture(u_Textures[7], uv);
    else if (texIndex == 8)  tex = texture(u_Textures[8], uv);
    else if (texIndex == 9)  tex = texture(u_Textures[9], uv);
    else if (texIndex == 10) tex = texture(u_Textures[10], uv);
    else if (texIndex == 11) tex = texture(u_Textures[11], uv);
    else if (texIndex == 12) tex = texture(u_Textures[12], uv);
    else if (texIndex == 13) tex = texture(u_Textures[13], uv);
    else if (texIndex == 14) tex = texture(u_Textures[14], uv);
    else if (texIndex == 15) tex = texture(u_Textures[15], uv);
    else if (texIndex == 16) tex = texture(u_Textures[16], uv);
    else if (texIndex == 17) tex = texture(u_Textures[17], uv);
    else if (texIndex == 18) tex = texture(u_Textures[18], uv);
    else if (texIndex == 19) tex = texture(u_Textures[19], uv);
    else if (texIndex == 20) tex = texture(u_Textures[20], uv);
    else if (texIndex == 21) tex = texture(u_Textures[21], uv);
    else if (texIndex == 22) tex = texture(u_Textures[22], uv);
    else if (texIndex == 23) tex = texture(u_Textures[23], uv);
    else if (texIndex == 24) tex = texture(u_Textures[24], uv);
    else if (texIndex == 25) tex = texture(u_Textures[25], uv);
    else if (texIndex == 26) tex = texture(u_Textures[26], uv);
    else if (texIndex == 27) tex = texture(u_Textures[27], uv);
    else if (texIndex == 28) tex = texture(u_Textures[28], uv);
    else if (texIndex == 29) tex = texture(u_Textures[29], uv);
    else if (texIndex == 30) tex = texture(u_Textures[30], uv);
    else if (texIndex == 31) tex = texture(u_Textures[31], uv);
    else tex = vec4(0,0,0,1); // fallback

    float msdf = median(tex.r, tex.g, tex.b);
    float sdf  = tex.a;
    return mix(msdf, sdf, u_MtsdfBlend);
}

void main()
{
    float sd = sampleDistance(v_TexCoord, v_TexIndex);

    float w = fwidth(sd);
    float alpha = smoothstep(0.5 - w - u_Softness,
                             0.5 + w + u_Softness,
                             sd);

    if (alpha <= 0.0)
        discard;

    float outline = 0.0;
    if (u_OutlineWidth > 0.0)
    {
        outline = smoothstep(
            0.5 - u_OutlineWidth - w,
            0.5 - u_OutlineWidth + w,
            sd
        );
    }

    float shadowAlpha = 0.0;
    if (length(u_ShadowOffset) > 0.0)
    {
        float shadowSD = sampleDistance(v_TexCoord + u_ShadowOffset, v_TexIndex);
        shadowAlpha = smoothstep(0.5 - w, 0.5 + w, shadowSD);
    }

    vec3 baseColor = v_Color.rgb;
    vec3 finalColor = baseColor;

    finalColor = mix(u_ShadowColor.rgb, finalColor, alpha);
    float finalAlpha = max(alpha, shadowAlpha * u_ShadowColor.a);

    if (u_OutlineWidth > 0.0)
    {
        finalColor = mix(u_OutlineColor.rgb, finalColor, alpha);
        finalAlpha = max(finalAlpha, outline);
    }

    finalAlpha *= v_Color.a;

    if (finalAlpha <= 0.0)
        discard;

    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    color = vec4(finalColor, finalAlpha);
    color2 = v_EntityID;
}