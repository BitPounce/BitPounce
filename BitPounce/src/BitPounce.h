#pragma once

#include "BitPounce/Audio/AudioDevice.h"
#include "BitPounce/Audio/Audio.h"

#include "BitPounce/Core/Application.h"
#include "BitPounce/Core/Layer.h"
#include "BitPounce/Core/Logger.h"
#include "BitPounce/Core/Window.h"
#include "BitPounce/Core/Input.h"
#include "BitPounce/Core/UUID.h"
#include "BitPounce/Core/Timestep.h"
#include "BitPounce/Core/MouseButtonCodes.h"
#include "BitPounce/Core/KeyCode.h"
#include "BitPounce/Core/PlatformTools.h"
#include <BitPounce/Core/FileSystem.h>
#include <BitPounce/Core/SystemManager.h>

#include "BitPounce/Events/Event.h"
#include "BitPounce/Events/ApplicationEvent.h"
#include "BitPounce/Events/KeyEvent.h"
#include "BitPounce/Events/MouseEvent.h"

#include "BitPounce/Renderer/Buffer.h"
#include "BitPounce/Renderer/GraphicsContext.h"
#include "BitPounce/Renderer/OrthographicCamera.h"
#include "BitPounce/Renderer/RenderCommand.h"
#include "BitPounce/Renderer/Renderer.h"
#include "BitPounce/Renderer/RendererAPI.h"
#include "BitPounce/Renderer/Shader.h"
#include "BitPounce/Renderer/VertexArray.h"
#include "BitPounce/Renderer/Texture.h"
#include "BitPounce/Renderer/OrthographicCameraController.h"
#include "BitPounce/Renderer/Renderer2D.h"
#include "BitPounce/Renderer/Framebuffer.h"
#include "BitPounce/Renderer/EditorCamera.h"

#include "BitPounce/Scene/Scene.h"
#include "BitPounce/Scene/Entity.h"
#include "BitPounce/Scene/Components.h"
#include "BitPounce/Scene/Systems/Renderer2DSystem.h"
#include "BitPounce/Scene/Systems/ScriptableEntitySystem.h"
#include "BitPounce/Scene/Systems/2DPhysicsSystem.h"
#include "BitPounce/Scene/Systems/CameraSystem.h"
#include "BitPounce/Scene/SceneSerializer.h"

#include "BitPounce/Scripting/ScriptEngine.h"

#include "BitPounce/Utils/PlatformUtils.h"

#include "BitPounce/Math/Math.h"