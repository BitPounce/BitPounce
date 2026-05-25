#include <bp_pch.h>
#include "Renderer3D.h"
#include <BitPounce/Math/Math.h>
#include "RenderCommand.h"

namespace BitPounce
{
	struct Renderer3DData
	{
		Renderer3D::Renderer3DSceneData sceneData;
		glm::mat4 matrix;
	};

	static Renderer3DData* s_Data;

	void Renderer3D::Init()
	{
		s_Data = new Renderer3DData();
	}

	void Renderer3D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer3D::BeginScene(const glm::mat4 &matrix, const Renderer3DSceneData& sceneData)
	{
		s_Data->matrix = matrix;
		s_Data->sceneData = sceneData;
	}
	
	void Renderer3D::EndScene()
	{
	}
	
	void Renderer3D::DrawModel(const glm::mat4 &tr, const Ref<Model> model, Ref<Material> shader)
	{
		shader->Bind();
		shader->SetMat4("u_CamMatrix", s_Data->matrix);
		shader->SetMat4("u_Model", tr);
		shader->SetFloat3(
			"u_LightPos",
			s_Data->sceneData.mainLightPos);
		shader->SetFloat4(
			"u_LightColour",
			s_Data->sceneData.mainLightColour);
		shader->SetFloat3(
			"u_CamPos",
			s_Data->sceneData.camPos);
		shader->SetFloat3(
			"u_LightDir",
			glm::vec3(-0.2f, -1.0f, -0.3f));

		Ref<Shader> rtyreytrytrytryt = shader->GetShader();
		RenderCommand::DrawIndexed(model, rtyreytrytrytryt);
	}
	
	void Renderer3D::DrawModel(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale, const Ref<Model> model, Ref<Material> shader)
	{
		glm::mat4 transform = Math::ComposeTransform(translation, rotation, scale);
		DrawModel(transform, model, shader);
	}
}
