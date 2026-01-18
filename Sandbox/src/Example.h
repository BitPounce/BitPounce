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
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		size_t size = 20000;
		test = (char*)DiskAlloc(size);
		for (size_t i = 0; i < size; i++)
		{
			test[i] = 'w';
		}
		
		DiskFree(size, test);


		m_VertexArray.reset(BitPounce::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		std::shared_ptr<BitPounce::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(BitPounce::VertexBuffer::Create(vertices, sizeof(vertices)));
		BitPounce::BufferLayout layout = {
			{ BitPounce::ShaderDataType::Float3, "a_Position" },
			{ BitPounce::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<BitPounce::IndexBuffer> indexBuffer;
		indexBuffer.reset(BitPounce::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(BitPounce::VertexArray::Create());

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		std::shared_ptr<BitPounce::VertexBuffer> squareVB;
		squareVB.reset(BitPounce::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ BitPounce::ShaderDataType::Float3, "a_Position" },
			{ BitPounce::ShaderDataType::Float2, "a_TexCoord" }
		});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<BitPounce::IndexBuffer> squareIB;
		squareIB.reset(BitPounce::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
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

		m_Shader.reset(BitPounce::Shader::Create(vertexSrc, fragmentSrc));

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

		m_TextureShader.reset(BitPounce::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));

		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
		m_TextureShader->Unbind();
	}

	void OnUpdate(BitPounce::Timestep& ts) override
	{
		if (BitPounce::Input::IsKeyPressed(BitPounce::Key::LeftArrow))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (BitPounce::Input::IsKeyPressed(BitPounce::Key::RightArrow))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;

		if (BitPounce::Input::IsKeyPressed(BitPounce::Key::UpArrow))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (BitPounce::Input::IsKeyPressed(BitPounce::Key::DownArrow))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (BitPounce::Input::IsKeyPressed(BitPounce::Key::A))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		if (BitPounce::Input::IsKeyPressed(BitPounce::Key::D))
			m_CameraRotation -= m_CameraRotationSpeed * ts;

		BitPounce::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		BitPounce::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		BitPounce::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<BitPounce::OpenGLShader>(m_TextureShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		m_Texture->Bind();
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				BitPounce::Renderer::Submit(m_TextureShader, m_SquareVA, transform);
			}
		}

		BitPounce::Renderer::Submit(m_Shader, m_VertexArray);


		BitPounce::Renderer::EndScene();
	}

	void OnEvent(BitPounce::Event& event) override
	{
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
		char* test;
		BitPounce::Ref<BitPounce::Shader> m_Shader;
		BitPounce::Ref<BitPounce::VertexArray> m_VertexArray;

		BitPounce::Ref<BitPounce::Shader> m_TextureShader;
		BitPounce::Ref<BitPounce::Texture2D> m_Texture;
		BitPounce::Ref<BitPounce::VertexArray> m_SquareVA;

		BitPounce::OrthographicCamera m_Camera;
		glm::vec3 m_CameraPosition;
		float m_CameraMoveSpeed = 5.0f;

		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 180.0f;
		glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

};