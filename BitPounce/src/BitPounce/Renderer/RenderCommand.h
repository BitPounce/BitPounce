#pragma once

#include "RendererAPI.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"

namespace BitPounce {

	class RenderCommand
	{
	public:

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			vertexArray->Bind();
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void DrawIndexed(const std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, uint32_t indexCount = 0)
		{
			mesh->Bind(shader);
			DrawIndexed(mesh->m_VertexArray, indexCount);
		}

		inline static void DrawIndexed(const std::shared_ptr<Model>& model, std::shared_ptr<Shader>& shader)
		{
			for (unsigned int i = 0; i < model->m_Meshes.size(); i++)
			{
				DrawIndexed(model->m_Meshes[i], shader);
			}
		}

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		static void SetLineWidth(float width)
		{
			s_RendererAPI->SetLineWidth(width);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};

}