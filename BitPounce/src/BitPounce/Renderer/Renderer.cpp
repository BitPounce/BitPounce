#include "bp_pch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace BitPounce
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;
	Ref<VertexArray> Renderer::s_FullScreenQuad = Ref<VertexArray>();

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
		Renderer3D::Init();
		s_FullScreenQuad = VertexArray::Create();

	    float QuadVertices[] = {
	        // Position        // UV
	        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom-left
	         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom-right
	         1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top-right
	        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top-left
	    };

	    auto QuadVertexBuffer = VertexBuffer::Create(QuadVertices, sizeof(QuadVertices));
	    QuadVertexBuffer->SetLayout({
	        { ShaderDataType::Float3, "a_Position" },
	        { ShaderDataType::Float2, "a_TexCoord" },
	    });
	    s_FullScreenQuad->AddVertexBuffer(QuadVertexBuffer);

	    uint32_t quadIndices[] = {
	        0, 1, 2,  // first triangle
	        2, 3, 0   // second triangle
	    };
	    auto indexBuffer = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
	    s_FullScreenQuad->SetIndexBuffer(indexBuffer);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

    void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}
	
	void Renderer::DrawFullScreenQuad()
    {
		s_FullScreenQuad->Bind();
		RenderCommand::DrawIndexed(s_FullScreenQuad);
    }

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}