#include "Enemy.h"

#include "../../main.h"
#include "../../Animation/AnimationPlayer.h"
#include "../Physics/GroundPhysics.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
Enemy::Enemy()
	:	m_upAnimationPlayer(std::make_unique<AnimationPlayer>()),
		m_upGroundPhysics(std::make_unique<GroundPhysics>())

{
}
Enemy::~Enemy() = default;

//===========================================================
// 初期化関数
//===========================================================
void Enemy::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();

		m_spModel->SetModelData(
			KdAssets::Instance().m_modeldatas.GetData(
				"Asset/Models/Enemy/Golem/Golem.gltf"
			)
		);
	}

	// 初期座標の設定
	SetPos({ 0.0f, 0.0f, 5.0f });
	UpdateWorldMatrix(GetPos());
}

//===========================================================
// 更新関数
//===========================================================
void Enemy::Update()
{
	const float	deltaTime	= Application::Instance().GetDeltaTime();
	const auto	player		= m_wpPlayer.lock();


	if (m_upAnimationPlayer && m_spModel)
	{
		m_upAnimationPlayer->Update(*m_spModel, deltaTime);
	}

	// 水平移動が終わったあと、最後に高さを確定する
	m_upGroundPhysics->Update(
		*this,
		deltaTime,
		0.0f
	);
}

//===========================================================
// 描画関数
//===========================================================
void Enemy::DrawLit()
{
	if (!m_spModel)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld
		);
}

//===========================================================
// ワールド行列更新関数
//===========================================================
void Enemy::UpdateWorldMatrix(
	const Math::Vector3&	pos,
	float					effectScaleXZ,
	float					effectScaleY
)
{
	const Math::Matrix scale = Math::Matrix::CreateScale(
		m_modelScale * effectScaleXZ,
		m_modelScale * effectScaleY,
		m_modelScale * effectScaleXZ
	);

	const Math::Matrix	rotation	= Math::Matrix::CreateRotationY(m_yaw);
	const Math::Matrix	translation = Math::Matrix::CreateTranslation(pos);

	m_mWorld = scale * rotation * translation;
}