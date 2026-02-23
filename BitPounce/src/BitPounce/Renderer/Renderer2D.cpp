#include "bp_pch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace BitPounce 
{
#if BP_RENDERER2D_USE_BATCH_RENDERING

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		float TexID = 0;
	};
	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> MainShader;
		Ref<Texture2D> WhiteTexture;
		
		uint32_t QuadIndexCount;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		Renderer2D::Renderer2DData RenderData;

		glm::vec4 QuadVertexPositions[4];
	};


	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		s_Data.QuadVertexArray = VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxQuads * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Colour" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,	  "a_TexID"}
		});
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
    		quadIndices[i + 0] = offset + 0;
    		quadIndices[i + 1] = offset + 1;
    		quadIndices[i + 2] = offset + 2;

    		quadIndices[i + 3] = offset + 2;
    		quadIndices[i + 4] = offset + 3;
    		quadIndices[i + 5] = offset + 0;

    		offset += 4;
		}

		Ref<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(squareIB);
		delete quadIndices;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}
			

		s_Data.MainShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.MainShader->Bind();
		s_Data.MainShader->SetInt("u_Texture", 0);
		s_Data.MainShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
	}

	void Renderer2D::Shutdown()
	{
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{

		s_Data.RenderData = Renderer2D::Renderer2DData();

		s_Data.MainShader->Bind();
		s_Data.MainShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::EndScene()
	{
		uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

    void Renderer2D::Flush()
    {
		s_Data.RenderData.RenderCalls++;
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);
		
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
		
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Tries += 2;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
		s_Data.QuadVertexBufferPtr->Colour = color; 
		s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
		s_Data.QuadVertexBufferPtr->Colour = color; 
		s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
		s_Data.QuadVertexBufferPtr->Colour = color; 
		s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
		s_Data.QuadVertexBufferPtr->Colour = color; 
		s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		#if 0
		s_Data.MainShader->SetFloat("m_TillingFactor", 1.0);
		s_Data.WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * 
		glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });


		s_Data.MainShader->SetMat4("u_Transform", transform);
		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
		#endif
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tillingFactor, const glm::vec4& tintColour)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tillingFactor, tintColour);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tillingFactor, const glm::vec4& tintColour)
	{
		int texIndex = 0;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { tillingFactor, 0.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { tillingFactor, tillingFactor };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, tillingFactor };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;


		#if 0
		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Tries += 2;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;
		s_Data.MainShader->SetFloat4("u_Color",  tintColour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data.MainShader->SetMat4("u_Transform", transform);
		s_Data.MainShader->SetFloat("m_TillingFactor", tillingFactor);

		texture->Bind();

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
		#endif
	}

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &colour)
    {
		DrawRotatedQuad({ position.x, position.y, 0 }, size, rotation, colour);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &colour)
    {
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                        * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;
	}

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const Ref<Texture2D> &texture, float tilingFactor, const glm::vec4& tintColour)
    {
		DrawRotatedQuad({position.x,position.y,0}, size, rotation, texture, tilingFactor, tintColour);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const Ref<Texture2D> &texture, float tilingFactor, const glm::vec4& tintColour)
    {
		int texIndex = 0;

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                        * glm::rotate(glm::mat4(1.0f), (rotation), glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { tilingFactor, 0.0f };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { tilingFactor, tilingFactor };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
		s_Data.QuadVertexBufferPtr->Colour = tintColour;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, tilingFactor };
		s_Data.QuadVertexBufferPtr->TexID = textureIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;
    }

#else
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
#endif

    Renderer2D::Renderer2DData Renderer2D::Get()
	{
    	return s_Data.RenderData;
	}
}