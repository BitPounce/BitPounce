#include "bp_pch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace BitPounce {

	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> MainShader;
		Ref<Texture2D> WhiteTexture;
		Renderer2D::Renderer2DData RenderData;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DStorage();
		s_Data->QuadVertexArray = VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> squareVB;
		squareVB =VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data->MainShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data->MainShader->Bind();
		s_Data->MainShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{

		s_Data->RenderData = Renderer2D::Renderer2DData();

		s_Data->MainShader->Bind();
		s_Data->MainShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		s_Data->RenderData.RenderCalls += 1;
		s_Data->RenderData.Quads += 1;
		s_Data->RenderData.Tries += 2;
		s_Data->RenderData.Vertices += 4;
		s_Data->RenderData.Indices += 6;
		s_Data->MainShader->SetFloat4("u_Color", color);
		s_Data->MainShader->SetFloat("m_TillingFactor", 1.0);
		s_Data->WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * 
		glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });


		s_Data->MainShader->SetMat4("u_Transform", transform);
		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tillingFactor, const glm::vec4& tintColour)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tillingFactor, tintColour);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tillingFactor, const glm::vec4& tintColour)
	{
		s_Data->RenderData.RenderCalls += 1;
		s_Data->RenderData.Quads += 1;
		s_Data->RenderData.Tries += 2;
		s_Data->RenderData.Vertices += 4;
		s_Data->RenderData.Indices += 6;
		s_Data->MainShader->SetFloat4("u_Color",  tintColour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->MainShader->SetMat4("u_Transform", transform);
		s_Data->MainShader->SetFloat("m_TillingFactor", tillingFactor);

		texture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &colour)
    {
		DrawRotatedQuad({ position.x, position.y, 0 }, size, rotation, colour);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &colour)
    {
		s_Data->MainShader->SetFloat4("u_Color", colour);
		s_Data->MainShader->SetFloat("m_TillingFactor", 1.0);
		s_Data->WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * 
		glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f }) * 
		glm::rotate(glm::mat4(1), rotation, {0,0,1});


		s_Data->MainShader->SetMat4("u_Transform", transform);
		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const Ref<Texture2D> &texture, float tilingFactor, const glm::vec4& tintColour)
    {
		DrawRotatedQuad({position.x,position.y,0}, size, rotation, texture, tilingFactor, tintColour);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const Ref<Texture2D> &texture, float tilingFactor, const glm::vec4& tintColour)
    {
		s_Data->RenderData.RenderCalls += 1;
		s_Data->RenderData.Quads += 1;
		s_Data->RenderData.Tries += 2;
		s_Data->RenderData.Vertices += 4;
		s_Data->RenderData.Indices += 6;
		s_Data->MainShader->SetFloat4("u_Color",  tintColour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1), rotation, {0,0,1}) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f }) ;
		s_Data->MainShader->SetMat4("u_Transform", transform);
		s_Data->MainShader->SetFloat("m_TillingFactor", tilingFactor);

		texture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    // TODO
	/*
    void Renderer2D::DrawQuad(const glm::mat4 transform, const glm::vec4 &color)
    {
		s_Data->RenderData.RenderCalls += 1;
		s_Data->RenderData.Quads += 1;
		s_Data->RenderData.Tries += 2;
		s_Data->RenderData.Vertices += 4;
		s_Data->RenderData.Indices += 6;

		s_Data->MainShader->SetFloat4("u_Color", color);
		s_Data->MainShader->SetFloat("m_TillingFactor", 1.0);
		s_Data->WhiteTexture->Bind();

		s_Data->MainShader->SetMat4("u_Transform", transform);
		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    void Renderer2D::DrawQuad(const glm::mat4 transform, const Ref<Texture2D> &texture, float tilingFactor)
    {
		s_Data->RenderData.RenderCalls += 1;
		s_Data->RenderData.Quads += 1;
		s_Data->RenderData.Tries += 2;
		s_Data->RenderData.Vertices += 4;
		s_Data->RenderData.Indices += 6;

		s_Data->MainShader->SetFloat4("u_Color",  glm::vec4(1.0f));

		s_Data->MainShader->SetMat4("u_Transform", transform);
		s_Data->MainShader->SetFloat("m_TillingFactor", tilingFactor);

		texture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }*/

    Renderer2D::Renderer2DData Renderer2D::Get()
	{
    	return s_Data->RenderData;
	}
}