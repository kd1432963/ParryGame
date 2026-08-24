#include "ArenaFloor.h"

//===========================================================
// 初期化関数
//===========================================================
void ArenaFloor::Init()
{
	if (!m_spModel)
	{
		m_spModel = KdAssets::Instance().m_modeldatas.GetData(
			"Asset/Models/Stage/ArenaFloor.gltf"
		);
	}

	// モデルの形状を、接地とカメラのめり込み防止判定として登録する
	if (m_spModel && !m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();

		m_pCollider->RegisterCollisionShape(
			"Ground",
			m_spModel,
			KdCollider::TypeGround | KdCollider::TypeSight
		);
	}

	// 拡大率をステージ用に設定
	SetScale(20.0f);
}

//===========================================================
// 描画関数
//===========================================================
void ArenaFloor::DrawLit()
{
	if (!m_spModel)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld
	);
}
