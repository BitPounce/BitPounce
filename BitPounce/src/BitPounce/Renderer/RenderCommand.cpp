#include "bp_pch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace BitPounce {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}