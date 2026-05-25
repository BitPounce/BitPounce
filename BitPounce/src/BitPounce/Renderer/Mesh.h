#pragma once

#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include <unordered_set>

namespace BitPounce
{
	struct MeshVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec4 colour;
		glm::vec2 uv;
	};

	

	class Mesh
	{
	public:
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);

		static Ref<Mesh> Create(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);
		void Bind(Ref<Shader> shader);
		Ref<VertexArray>& GetVertexArray() { return m_VertexArray; }

		constexpr bool operator==(const Mesh& other) const
		{
			return m_ID == other.m_ID;
		}
	private:
		UUID m_ID;
		std::vector<MeshVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Ref<VertexArray> m_VertexArray;
		friend class RenderCommand;
	};
}