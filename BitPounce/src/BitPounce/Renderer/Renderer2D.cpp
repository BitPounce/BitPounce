#include "bp_pch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include "MSDFData.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace BitPounce 
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		float TexID = 0;

		// Editor-only
		int EntityID;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

		// Editor-only
		int EntityID;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		int EntityID;
	};

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		int TexID = 0;

		// TODO: bg color for outline/bg


		// Editor-only
		int EntityID;
	};

	struct Frustum
	{
		glm::vec4 planes[6]; // left, right, bottom, top, near, far
	};
	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Texture2D> WhiteTexture;

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;

		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		Ref<VertexArray> TextVertexArray;
		Ref<VertexBuffer> TextVertexBuffer;
		Ref<Shader> TextShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		uint32_t TextIndexCount = 0;
		TextVertex* TextVertexBufferBase = nullptr;
		TextVertex* TextVertexBufferPtr = nullptr;

		float LineWidth = 2.0f;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		std::array<Ref<Texture2D>, MaxTextureSlots> FontTextureSlots;
		uint32_t TextureSlotIndex = 1;
		uint32_t FontTextureSlotIndex = 0;

		Ref<Texture2D> FontAtlasTexture;

		Renderer2D::Renderer2DData RenderData;

		glm::vec4 QuadVertexPositions[4];

		Frustum frustum;

	};

	static Renderer2DData s_Data;

	Frustum ExtractFrustum(const glm::mat4& m)
	{
		Frustum f;
		glm::vec4 row0 = glm::row(m, 0);
		glm::vec4 row1 = glm::row(m, 1);
		glm::vec4 row2 = glm::row(m, 2);
		glm::vec4 row3 = glm::row(m, 3);

		f.planes[0] = row3 + row0;   // left
		f.planes[1] = row3 - row0;   // right
		f.planes[2] = row3 + row1;   // bottom
		f.planes[3] = row3 - row1;   // top
		f.planes[4] = row3 + row2;   // near
		f.planes[5] = row3 - row2;   // far

		// Normalize
		for (int i = 0; i < 6; ++i)
		{
			float len = glm::length(glm::vec3(f.planes[i]));
			f.planes[i] /= len;
		}
		return f;
	}

	static bool ObstructionCulling(const glm::mat4& model, const glm::vec3& localMin = {-0.5f, -0.5f, -0.5f}, const glm::vec3& localMax = { 0.5f,  0.5f,  0.5f})
	{
		glm::vec3 center = (localMin + localMax) * 0.5f;
		glm::vec3 extents = (localMax - localMin) * 0.5f;

		glm::vec3 worldCenter = glm::vec3(model * glm::vec4(center, 1.0f));
		glm::mat3 absM = glm::mat3(glm::abs(model[0]), glm::abs(model[1]), glm::abs(model[2]));
		glm::vec3 worldExtents = absM * extents;

		for (int i = 0; i < 6; ++i)
		{
			glm::vec3 normal = glm::vec3(s_Data.frustum.planes[i]);
			float distance = glm::dot(normal, worldCenter) + s_Data.frustum.planes[i].w;
			float radius = glm::dot(worldExtents, glm::abs(normal));
			if (distance < -radius)
				return false;   // completely outside
		}
		return true;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::Init()
	{
		s_Data.QuadVertexArray = VertexArray::Create();
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Colour" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexID" },
			{ ShaderDataType::Int,    "a_EntityID"     }
		});
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

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
		Ref<IndexBuffer> squareIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(squareIB);
		delete[] quadIndices;

		// Circles
		s_Data.CircleVertexArray = VertexArray::Create();

		s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
		s_Data.CircleVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_WorldPosition" },
			{ ShaderDataType::Float3, "a_LocalPosition" },
			{ ShaderDataType::Float4, "a_Color"         },
			{ ShaderDataType::Float,  "a_Thickness"     },
			{ ShaderDataType::Float,  "a_Fade"          },
			{ ShaderDataType::Int,    "a_EntityID"      }
		});
		s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
		s_Data.CircleVertexArray->SetIndexBuffer(squareIB); // Use quad IB
		s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

		// Lines
		s_Data.LineVertexArray = VertexArray::Create();

		s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
		s_Data.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Int,    "a_EntityID" }
		});
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
		s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

		s_Data.TextVertexArray = VertexArray::Create();

		s_Data.TextVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(TextVertex));
		s_Data.TextVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoord"     },
			{ ShaderDataType::Int,  "a_TexID" },
			{ ShaderDataType::Int,    "a_EntityID"     }
		});
		s_Data.TextVertexArray->AddVertexBuffer(s_Data.TextVertexBuffer);
		s_Data.TextVertexArray->SetIndexBuffer(squareIB);
		s_Data.TextVertexBufferBase = new TextVertex[s_Data.MaxVertices];

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.QuadShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.CircleShader = Shader::Create("assets/shaders/Circle.glsl");
		s_Data.LineShader = Shader::Create("assets/shaders/Line.glsl");
		s_Data.TextShader = Shader::Create("assets/shaders/Text.glsl");
		s_Data.QuadShader->Bind();
		s_Data.QuadShader->SetInt("u_Texture", 0);
		s_Data.QuadShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
	}

	void Renderer2D::Shutdown()
	{
		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const glm::mat4 &matrix)
	{
		s_Data.RenderData = Renderer2D::Renderer2DData();
		s_Data.QuadShader->Bind();
		s_Data.QuadShader->SetMat4("u_ViewProjection", matrix);
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.TextureSlotIndex = 1;

		for (int i = 0; i < s_Data.MaxVertices; i++)
		{
			s_Data.QuadVertexBufferPtr[i].EntityID = -1;
		}

		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
		s_Data.CircleShader->Bind();
		s_Data.CircleShader->SetMat4("u_ViewProjection", matrix);

		s_Data.LineShader->Bind();
		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;	
		
		s_Data.TextIndexCount = 0;
		s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;
		s_Data.TextShader->Bind();
		s_Data.TextShader->SetMat4("u_ViewProjection", matrix);
		s_Data.FontTextureSlotIndex = 0;

		s_Data.frustum = ExtractFrustum(matrix);
		
	}

	static bool FrustumCulling(const glm::mat4& mat)
	{

	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		glm::mat4 viewProj = camera.GetViewProjection();
		BeginScene(viewProj);
	}

	void Renderer2D::BeginScene(const Camera &camera, const glm::mat4 &transform)
	{
		BeginScene(camera.GetProjection() * glm::inverse(transform));
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		BeginScene(camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{
		
		Flush();
	}

	void Renderer2D::Flush()
	{
		if (true)
		{
			s_Data.QuadShader->Bind();
			uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
			s_Data.RenderData.RenderCalls++;
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		}

		if (s_Data.CircleIndexCount)
		{
			s_Data.CircleShader->Bind();
			uint32_t dataSize = (uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase;
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);
			s_Data.RenderData.RenderCalls++;

			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		}

		if (s_Data.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

			s_Data.LineShader->Bind();
			RenderCommand::SetLineWidth(s_Data.LineWidth);
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
			s_Data.RenderData.RenderCalls++;
		}

		if (s_Data.TextIndexCount)
		{
		    uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TextVertexBufferPtr - (uint8_t*)s_Data.TextVertexBufferBase);
		    s_Data.TextVertexBuffer->SetData(s_Data.TextVertexBufferBase, dataSize);
		
		    // Bind all used font textures
		    for (uint32_t i = 0; i < s_Data.FontTextureSlotIndex; i++)
		        s_Data.FontTextureSlots[i]->Bind(i);
		
		    s_Data.TextShader->Bind();
		    // Set the texture array uniform (once, could be done in Init)
		    int samplers[s_Data.MaxTextureSlots];
		    for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
		        samplers[i] = i;
		    s_Data.TextShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		
		    RenderCommand::DrawIndexed(s_Data.TextVertexArray, s_Data.TextIndexCount);
		    s_Data.RenderData.RenderCalls++;
		}
	}

	// Colour quads
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawString(const std::string & string, Ref<Font> font, const glm::mat4 & transform, const TextParams& textParams, int entityID)
	{
		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();
		Ref<Texture2D> fontAtlas = font->GetAtlasTexture();

		s_Data.FontAtlasTexture = fontAtlas;

		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;

		const float spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();
		
		for (size_t i = 0; i < string.size(); i++)
		{
			char character = string[i];
			if (character == '\r')
				continue;

			if (character == '\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + textParams.LineSpacing;
				continue;
			}

			if (character == ' ')
			{
				float advance = spaceGlyphAdvance;
				if (i < string.size() - 1)
				{
					char nextCharacter = string[i + 1];
					double dAdvance;
					fontGeometry.getAdvance(dAdvance, character, nextCharacter);
					advance = (float)dAdvance;
				}

				x += fsScale * advance + textParams.Kerning;
				continue;
			}

			if (character == '\t')
			{
				// is this right idk?
				x += 4.0f * (fsScale * spaceGlyphAdvance + textParams.Kerning);
				continue;
			}

			auto glyph = fontGeometry.getGlyph(character);
			if (!glyph)
				glyph = fontGeometry.getGlyph('?');
			if (!glyph)
				return;

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 texCoordMin((float)al, (float)ab);
			glm::vec2 texCoordMax((float)ar, (float)at);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin((float)pl, (float)pb);
			glm::vec2 quadMax((float)pr, (float)pt);

			quadMin *= fsScale, quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / fontAtlas->GetWidth();
			float texelHeight = 1.0f / fontAtlas->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			glm::mat4 glyphTransform = transform *
				glm::translate(glm::mat4(1.0f), glm::vec3(quadMin, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(quadMax - quadMin, 1.0f));

			if(!ObstructionCulling(glyphTransform))
			{
				if (i < string.size() - 1)
				{
					double advance = glyph->getAdvance();
					char nextCharacter = string[i + 1];
					fontGeometry.getAdvance(advance, character, nextCharacter);
				
					x += fsScale * advance + textParams.Kerning;
				}
				continue;
			}

			float textureIndex = -1.0f;
			for (uint32_t i = 0; i < s_Data.FontTextureSlotIndex; i++)
			{
			    if (*s_Data.FontTextureSlots[i] == *fontAtlas)
			    {
			        textureIndex = (float)i;
			        break;
			    }
			}
			if (textureIndex < 0.0f)
			{
			    textureIndex = (float)s_Data.FontTextureSlotIndex;
			    s_Data.FontTextureSlots[s_Data.FontTextureSlotIndex] = fontAtlas;
			    s_Data.FontTextureSlotIndex++;
			}

			// render here
			s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
			s_Data.TextVertexBufferPtr->Color = textParams.Colour;
			s_Data.TextVertexBufferPtr->TexID = (int)textureIndex;
			s_Data.TextVertexBufferPtr->TexCoord = texCoordMin;
			s_Data.TextVertexBufferPtr->EntityID = entityID;
			s_Data.TextVertexBufferPtr++;

			s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
			s_Data.TextVertexBufferPtr->Color = textParams.Colour;
			s_Data.TextVertexBufferPtr->TexCoord = { texCoordMin.x, texCoordMax.y };
			s_Data.TextVertexBufferPtr->TexID = (int)textureIndex;
			s_Data.TextVertexBufferPtr->EntityID = entityID;
			s_Data.TextVertexBufferPtr++;

			s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
			s_Data.TextVertexBufferPtr->Color = textParams.Colour;
			s_Data.TextVertexBufferPtr->TexID = (int)textureIndex;
			s_Data.TextVertexBufferPtr->TexCoord = texCoordMax;
			s_Data.TextVertexBufferPtr->EntityID = entityID;
			s_Data.TextVertexBufferPtr++;

			s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
			s_Data.TextVertexBufferPtr->Color = textParams.Colour;
			s_Data.TextVertexBufferPtr->TexID = (int)textureIndex;
			s_Data.TextVertexBufferPtr->TexCoord = { texCoordMax.x, texCoordMin.y };
			s_Data.TextVertexBufferPtr->EntityID = entityID;
			s_Data.TextVertexBufferPtr++;

			s_Data.TextIndexCount += 6;
			s_Data.RenderData.Quads++;

			if (i < string.size() - 1)
			{
				double advance = glyph->getAdvance();
				char nextCharacter = string[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter);

				x += fsScale * advance + textParams.Kerning;
			}
		}
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		if(!ObstructionCulling(transform))
		{
			return;
		}

		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = color;
			s_Data.QuadVertexBufferPtr->TexCoord = { i == 1 || i == 2 ? 1.0f : 0.0f, i >= 2 ? 1.0f : 0.0f };
			s_Data.QuadVertexBufferPtr->TexID = 0.0f;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
	}

	// Textured quads
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColour)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColour);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColour)
	{
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
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
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		if(!ObstructionCulling(transform))
		{
			return;
		}

		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;

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

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		if(!ObstructionCulling(transform))
		{
			return;
		}

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = colour;
			s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(1); // I DO NOT CARE
			s_Data.QuadVertexBufferPtr->TexID = 0;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColour, float tilingFactor , int entityID)
	{
		if(!ObstructionCulling(transform))
		{
			return;
		}
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
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

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = tintColour;
			s_Data.QuadVertexBufferPtr->TexCoord = { i == 1 || i == 2 ? tilingFactor : 0.0f, i >= 2 ? tilingFactor : 0.0f };
			s_Data.QuadVertexBufferPtr->TexID = textureIndex;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
	}

    void Renderer2D::DrawQuad(const glm::mat4 &transform, const Ref<Texture2D> &texture, const glm::u32vec2 subTexSize, const glm::u32vec2 subTexIndex, const glm::vec4 &tintColour, int entityID)
    {
		glm::u32vec2 texSize = glm::u32vec2(texture->GetWidth(), texture->GetHeight());

    	// Compute pixel coordinates of the four corners of the sub‑texture
    	glm::u32vec2 a = subTexIndex * subTexSize;                               // top‑left
    	glm::u32vec2 b = glm::u32vec2((subTexIndex.x + 1) * subTexSize.x, subTexIndex.y * subTexSize.y);            // top‑right
    	glm::u32vec2 c = (subTexIndex + glm::u32vec2(1, 1)) * subTexSize;        // bottom‑right
    	glm::u32vec2 e = glm::u32vec2(subTexIndex.x * subTexSize.x, (subTexIndex.y + 1) * subTexSize.y);      // bottom‑left

		std::array<glm::vec2, 4> uvs = {a,b,c,e};

		for (int i = 0; i < 4; i++)
		{
			uvs[i] /= texSize;
		}

		DrawQuad(transform, texture, uvs, tintColour, entityID);
    }

    void Renderer2D::DrawQuad(const glm::mat4 &transform, const Ref<Texture2D> &texture, const std::array<glm::vec2, 4> uvs, const glm::vec4 &tintColour, int entityID)
    {
		if(!ObstructionCulling(transform))
		{
			return;
		}
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		s_Data.RenderData.Quads += 1;
		s_Data.RenderData.Vertices += 4;
		s_Data.RenderData.Indices += 6;

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
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

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = tintColour;
			s_Data.QuadVertexBufferPtr->TexCoord = uvs[i];
			s_Data.QuadVertexBufferPtr->TexID = textureIndex;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &colour)
	{
		DrawRotatedQuad(glm::vec3(position, 0), size, rotation, colour);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &colour)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f})
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, colour);


	}

	// Rotated textured quads (radians)
	void Renderer2D::DrawRotatedQuad( const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColour)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColour);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColour)
	{
		
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
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
			* glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f})
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
			
		if(!ObstructionCulling(transform))
		{
			return;
		}
		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = tintColour;
			s_Data.QuadVertexBufferPtr->TexCoord = { i == 1 || i == 2 ? tilingFactor : 0.0f, i >= 2 ? tilingFactor : 0.0f };
			s_Data.QuadVertexBufferPtr->TexID = textureIndex;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID)
	{
		s_Data.LineVertexBufferPtr->Position = p0;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = p1;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
	{
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

		DrawLine(p0, p1, color);
		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p0, color);
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		if(!ObstructionCulling(transform))
		{
			return;
		}
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * s_Data.QuadVertexPositions[i];

		DrawLine(lineVertices[0], lineVertices[1], color);
		DrawLine(lineVertices[1], lineVertices[2], color);
		DrawLine(lineVertices[2], lineVertices[3], color);
		DrawLine(lineVertices[3], lineVertices[0], color);
	}

	void Renderer2D::DrawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade, int entityID)
	{
		if(!ObstructionCulling(transform))
		{
			return;
		}

		for (size_t i = 0; i < 4; i++)
		{
			s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
			s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = entityID;
			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += 6;

		s_Data.RenderData.Quads++;
	}

	float Renderer2D::GetLineWidth()
	{
		return s_Data.LineWidth;
	}

	void Renderer2D::SetLineWidth(float width)
	{
		s_Data.LineWidth = width;
	}

	Renderer2D::Renderer2DData Renderer2D::Get()
	{
		return s_Data.RenderData;
	}


}