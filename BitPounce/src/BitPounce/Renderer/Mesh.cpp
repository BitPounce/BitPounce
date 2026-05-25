#include <bp_pch.h>
#include "Mesh.h"

namespace BitPounce
{
	Mesh::Mesh(const std::vector<MeshVertex> &vertices, const std::vector<uint32_t> &indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		m_VertexArray = VertexArray::Create();
		m_VertexArray->Bind();
		auto vertexBuffer = VertexBuffer::Create(reinterpret_cast<float*>(m_Vertices.data()), vertices.size() * sizeof(MeshVertex));
		vertexBuffer->SetLayout({
			{ BitPounce::ShaderDataType::Float3, "a_Position" },
			{ BitPounce::ShaderDataType::Float3, "a_Normal"},
			{ BitPounce::ShaderDataType::Float4, "a_Colour"},
			{ BitPounce::ShaderDataType::Float2, "a_UV"}
		});

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		auto indicesBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
		m_VertexArray->SetIndexBuffer(indicesBuffer);
	}

	Ref<Mesh> Mesh::Create(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
	{
		return CreateRef<Mesh>(vertices, indices);
	}
	void Mesh::Bind(Ref<Shader> shader)
	{
		//shader->Bind();
		m_VertexArray->Bind();
	
		/*
		std::unordered_map<TextureType, uint32_t> numTypeMap;
	
		for (uint8_t i = 0; i < (uint8_t)TextureType::Count; i++)
		{
			numTypeMap[(TextureType)i] = 0;
		}
	
		for (uint32_t i = 0; i < textures.size(); i++)
		{
			const TexPair& pair = textures[i];
		
			TextureType typeEnum = pair.first;
		
			std::string type = TextureTypeToString(typeEnum);
			std::string num  = std::to_string(numTypeMap[typeEnum]++);
		
			std::string uniformName = "u_" + type + num;
		
			pair.second->Bind(i);
			shader->SetInt(uniformName, i);
		}*/
	}
}