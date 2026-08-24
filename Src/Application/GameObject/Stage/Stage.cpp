#include "Stage.h"

void Stage::Init()
{
	if (!m_spModel)
	{
		m_spModel = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Stage/Stage.gltf");
	}

	SetPos({ 0.0f, -1.0f, 0.0f });
	SetScale(20.0f);
}

void Stage::DrawLit()
{
	if (!m_spModel)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
