#include <stddef.h>

#if defined(__EMSCRIPTEN__)
    #include <wasm_simd128.h>
#elif defined(__AVX2__)
    #include <immintrin.h>
#elif defined(__SSE2__)
    #include <emmintrin.h>
#endif

void* DiskMemCpy(void* dest, const void* src, size_t size)
{
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

#if defined(__EMSCRIPTEN__)

    // WASM SIMD (128-bit)
    while (size >= 16)
    {
        v128_t v = wasm_v128_load(s);
        wasm_v128_store(d, v);
        s += 16;
        d += 16;
        size -= 16;
    }

#elif defined(__AVX2__)

    // AVX2 (256-bit)
    while (size >= 32)
    {
        __m256i v = _mm256_loadu_si256((const __m256i*)s);
        _mm256_storeu_si256((__m256i*)d, v);
        s += 32;
        d += 32;
        size -= 32;
    }

    _mm256_zeroupper(); // IMPORTANT

#elif defined(__SSE2__)

    // SSE2 (128-bit)
    while (size >= 16)
    {
        __m128i v = _mm_loadu_si128((const __m128i*)s);
        _mm_storeu_si128((__m128i*)d, v);
        s += 16;
        d += 16;
        size -= 16;
    }

#endif

    // Scalar tail
    while (size--)
        *d++ = *s++;

    return dest;
}