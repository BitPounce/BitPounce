#type vertex
#include "BasicVextex.glsl"

#type fragment

in vec2 v_TexCoord;
in vec3 v_Position;

layout(location = 0) out vec4 colour;

uniform sampler2D u_Texture;   // must be an sRGB texture (GL_SRGB8)

// Blur parameters (same as original)
const float SIGMA = 2.5;
const int KERNEL_RADIUS = 8;    // ceil(3 * SIGMA)

// ------------------------------------------------------------------
// Precomputed 1D Gaussian weights for offsets -R .. R
// (calculated at shader compile time, no exp() at runtime)
// ------------------------------------------------------------------
const float GAUSS_WEIGHTS[17] = float[](
    0.004402,   // dx = -8
    0.008788,   // -7
    0.016043,   // -6
    0.026786,   // -5
    0.040905,   // -4
    0.057128,   // -3
    0.072977,   // -2
    0.085269,   // -1
    0.091140,   //  0
    0.085269,   //  1
    0.072977,   //  2
    0.057128,   //  3
    0.040905,   //  4
    0.026786,   //  5
    0.016043,   //  6
    0.008788,   //  7
    0.004402    //  8
);

// Precomputed sum of the 1D weights = 0.7401 (approx)
const float SUM_1D = 0.740122;  // sum(GAUSS_WEIGHTS)
const float TOTAL_WEIGHT = SUM_1D * SUM_1D;  // = 0.5478

// ------------------------------------------------------------------
// sRGB -> Linear (only for the final output conversion)
// ------------------------------------------------------------------
vec3 linear_to_srgb(vec3 linear)
{
    return pow(linear, vec3(1.0 / 2.2));
}

float luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    ivec2 texSize = textureSize(u_Texture, 0);
    vec2 texelSize = 1.0 / vec2(texSize);

    // Original texel colour (sRGB) – will be used later in mix()
    vec2 uv = v_TexCoord * 2.0 - 1.0;
    vec2 offset = uv.yx / 10.0;
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    vec3 texColour_srgb = texture(u_Texture, uv).rgb;
    

    vec3 accumLinear = vec3(0.0);

    // Blur kernel
    for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
    {
        float weightY = GAUSS_WEIGHTS[y + KERNEL_RADIUS];
        vec2 offsetY = vec2(0.0, float(y) * texelSize.y);

        for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
        {
            float weight = weightY * GAUSS_WEIGHTS[x + KERNEL_RADIUS];
            vec2 sampleUV = uv + offsetY + vec2(float(x) * texelSize.x, 0.0);

            // texture() returns linear because u_Texture is sRGB format
            vec3 sampleLinear = texture(u_Texture, sampleUV).rgb;

            accumLinear += sampleLinear * weight;
        }
    }

    vec3 blurredLinear = accumLinear / TOTAL_WEIGHT;
    vec3 blurredSRGB = linear_to_srgb(blurredLinear);
    float brightness = luma(blurredSRGB);
    vec3 outColour = mix(texColour_srgb, blurredSRGB, brightness);
    if (uv.x <= 0.0 || 1.0 <= uv.x || uv.y <= 0.0 || 1.0 <= uv.y)
        outColour = vec3(0.0);
    uv = uv * 2.0 - 1.0;
    vec2 vignette = 30.0 / vec2(texSize);
    vignette = smoothstep(vec2(0.0), vignette, vec2(1.0) - abs(uv));
    vignette = clamp(vignette, vec2(0.0), vec2(1.0));
    outColour.g *= (sin(v_TexCoord.y * float(texSize.y) * 2.0) + 1.0) * 0.15 + 1.0;
    outColour.rb *= vec2(
    (cos(v_TexCoord.y * float(texSize.y) * 2.0) + 1.0) * 0.135 + 1.0
);

    colour = vec4(clamp(outColour, 0.0, 1.0) * vignette.x * vignette.y, 1.0);
}