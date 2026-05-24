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
		using TexPair = std::pair<TextureType, Ref<Texture2D>>;
	public:
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<TexPair>& textures);

		std::vector<TexPair> textures;
		static Ref<Mesh> Create(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<TexPair>& textures);
		void Bind(Ref<Shader> shader);
		Ref<VertexArray>& GetVertexArray() { return m_VertexArray; }
	private:
		std::vector<MeshVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Ref<VertexArray> m_VertexArray;
		friend class RenderCommand;
	};
}