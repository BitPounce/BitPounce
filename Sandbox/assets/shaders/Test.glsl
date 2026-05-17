#type vertex
#include "BasicVextex.glsl"

#type fragment

in vec2 v_TexCoord;
in vec3 v_Position;

layout(location = 0) out vec4 colour;

uniform sampler2D u_Texture;   // GL_SRGB8 texture
uniform vec4 u_Colour;

int u_Mode= 0; 
// 0 = Bilateral blur (edge-preserving)
// 1 = No blur (original warped texture)

// -----------------------------
// Settings
// -----------------------------
uniform int KERNEL_RADIUS;
// x = sin time
// y = time
uniform vec3 u_Time;

// -----------------------------
// Helpers
// -----------------------------
vec3 linear_to_srgb(vec3 c)
{
    return mix(
        12.92 * c,
        1.055 * pow(c, vec3(1.0/2.4)) - 0.055,
        step(0.0031308, c)
    );
}

float luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}


float hash(vec2 p) {
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f); // smooth interpolation

    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

// -----------------------------
// Bilateral Blur (ONLY BLUR IMPLEMENTATION)
// -----------------------------
vec3 bilateralBlur(vec2 uv, vec2 texelSize)
{
    vec3 centerColor = texture(u_Texture, uv).rgb;

    // MUCH blurrier
    float sigmaS = 8.0;     
    float sigmaR = 0.35;

    float twoSigmaS2 = 2.0 * sigmaS * sigmaS;
    float twoSigmaR2 = 2.0 * sigmaR * sigmaR;

    vec3 accum = vec3(0.0);
    float weightSum = 0.0;

    for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
    {
        for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;

            vec3 sampleColor =
                texture(u_Texture, uv + offset).rgb;

            // spatial blur weight
            float ds2 = float(x*x + y*y);
            float wS = exp(-ds2 / twoSigmaS2);

            // edge preserving weight
            vec3 diff = sampleColor - centerColor;
            float dr2 = dot(diff, diff);
            float wR = exp(-dr2 / twoSigmaR2);
            if (wS * wR < 0.001) continue;
            float w = wS * wR;

            accum += sampleColor * w;
            weightSum += w;
        }
    }

    return accum / weightSum;
}

// -----------------------------
// Main
// -----------------------------
void main()
{
    ivec2 texSize = textureSize(u_Texture, 0);
    vec2 texelSize = 1.0 / vec2(texSize);

    // UV warp (your original distortion)
    vec2 uv = v_TexCoord * 2.0 - 1.0;
    vec2 offset = uv.yx / 10.0;
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;

    vec3 original = linear_to_srgb(texture(u_Texture, uv).rgb);
    

    vec3 processed;

    if (u_Mode == 0)
    {
        processed = bilateralBlur(uv, texelSize);
    }
    else
    {
        processed = original;
    }

    vec3 processedSRGB = linear_to_srgb(processed);

    float brightness = luma(processedSRGB);

    vec3 outColour = mix(original, processedSRGB,clamp(brightness + 0.5, 0.0, 1.0) );
    outColour *= u_Colour.rgb;
    //outColour.r /= 1.3;
    //outColour.g /= 1.2;
    //outColour.b /= 1.2;
    //outColour.r *= mix(1.0, 1.1, uv.y);
    //outColour.g /= mix(1.0, 1.1, uv.x);
    //outColour.b *= mix(1.0, 1.1, uv.y);
    //outColour.r *= mix(1.0, 1.9, fbm(vec2(uv.x, uv.y) * outColour.r));
    //outColour.g *= mix(1.0, 1.9, fbm(vec2(uv.y, uv.x) * outColour.g));
    //outColour.b *= mix(1.0, 1.9, fbm(vec2(uv.y, uv.y) * outColour.b));

    // Out-of-bounds kill
    if (uv.x <= 0.0 || uv.x >= 1.0 || uv.y <= 0.0 || uv.y >= 1.0)
        outColour = vec3(0.0);

    // Vignette
    vec2 uv2 = uv * 2.0 - 1.0;
    vec2 vignette = 30.0 / vec2(texSize);
    vignette = smoothstep(vec2(0.0), vignette, vec2(1.0) - abs(uv2));
    vignette = clamp(vignette, vec2(0.0), vec2(1.0));

    // Scanline-style chromatic modulation (your original look)
    outColour.g *= (sin(v_TexCoord.y * float(texSize.y) * 2.0) + 1.0) * 0.15 + 1.0;

    outColour.rb *= vec2(
        (cos(v_TexCoord.y * float(texSize.y) * 2.0) + 1.0) * 0.135 + 1.0
    );

    outColour.r = clamp(outColour.r, 0.0, 1.0);
    outColour.g = clamp(outColour.g, 0.0, 1.0);
    outColour.b = clamp(outColour.b, 0.0, 1.0);

    colour = vec4(clamp(outColour, 0.0, 1.0) * vignette.x * vignette.y, 1.0);
}