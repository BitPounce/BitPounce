#pragma once
#include <BitPounce.h>
#include <imgui.h>

#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class ExampleLayer : public BitPounce::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f / 720.0f)
	{

		BitPounce::FileSystem::AddFile("assets/textures/Checkerboard.png");


		m_VertexArray = BitPounce::VertexArray::Create();

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		std::shared_ptr<BitPounce::VertexBuffer> vertexBuffer;
		vertexBuffer = BitPounce::VertexBuffer::Create(vertices, sizeof(vertices));
		BitPounce::BufferLayout layout = {
			{ BitPounce::ShaderDataType::Float3, "a_Position" },
			{ BitPounce::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<BitPounce::IndexBuffer> indexBuffer;
		indexBuffer =BitPounce::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA = BitPounce::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		std::shared_ptr<BitPounce::VertexBuffer> squareVB;
		squareVB =BitPounce::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
			{ BitPounce::ShaderDataType::Float3, "a_Position" },
			{ BitPounce::ShaderDataType::Float2, "a_TexCoord" }
		});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<BitPounce::IndexBuffer> squareIB;
		squareIB =BitPounce::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(#version 300 es
			precision mediump float;
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string fragmentSrc = R"(#version 300 es
			precision mediump float;
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader = BitPounce::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

		std::string textureShaderVertexSrc = R"(#version 300 es
			precision mediump float;
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;

			out vec3 v_Position;
			out vec2 v_TexCoord;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				v_TexCoord = a_TexCoord;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string textureShaderFragmentSrc = R"(#version 300 es
			precision mediump float;
			
			layout(location = 0) out vec4 color;
		
			in vec2 v_TexCoord;
			in vec3 v_Position;

			uniform vec3 u_Color;
			uniform sampler2D u_Texture;

			void main()
			{
				color = texture(u_Texture, v_TexCoord) * vec4(u_Color, 1.0);
			}
		)";

		m_Texture = BitPounce::Texture2D::Create("assets/textures/Checkerboard.png");
		m_PlayerTexture = BitPounce::Texture2D::Create("assets/textures/Player.png");

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
		textureShader->Unbind();
	}

	void OnUpdate(BitPounce::Timestep& ts) override
	{
		m_CameraController.OnUpdate(ts);

		BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		BitPounce::RenderCommand::Clear();


		BitPounce::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		auto textureShader = m_ShaderLibrary.Get("Texture");

		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(textureShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		m_Texture->Bind();
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				BitPounce::Renderer::Submit(textureShader, m_SquareVA, transform);
			}
		}

		BitPounce::Renderer::Submit(m_Shader, m_VertexArray);
		m_PlayerTexture->Bind();
		BitPounce::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		BitPounce::Renderer::EndScene();
	}

	void OnEvent(BitPounce::Event& event) override
	{
		m_CameraController.OnEvent(event);
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Debug");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0/(ImGui::GetIO().Framerate),(ImGui::GetIO().Framerate));
		ImGui::End();

		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	private:
		
		BitPounce::ShaderLibrary m_ShaderLibrary;
		BitPounce::Ref<BitPounce::Shader> m_Shader;
		BitPounce::Ref<BitPounce::VertexArray> m_VertexArray;

		BitPounce::Ref<BitPounce::Texture2D> m_Texture, m_PlayerTexture;
		BitPounce::Ref<BitPounce::VertexArray> m_SquareVA;

		BitPounce::OrthographicCameraController m_CameraController;

		glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

};