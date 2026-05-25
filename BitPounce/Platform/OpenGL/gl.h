#pragma once

#ifdef BP_PLATFORM_WEB
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#include <GL/gl.h>   // fallback for constants
#endif // BP_PLATFORM_WEB
