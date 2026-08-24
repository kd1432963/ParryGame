#include "Player.h"

#include "../../main.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
Player::Player()	= default;
Player::~Player()	= default;

//===========================================================
// 初期化関数
//===========================================================
void Player::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData(
			KdAssets::Instance().m_modeldatas.GetData(
				"Asset/Models/Player/Player.gltf")
		);
	}
}

//===========================================================
// 更新関数
//===========================================================
void Player::Update()
{
	// デルタタイム取得
	float deltaTime		= Application::Instance().GetDeltaTime();

	UpdateYaw(deltaTime);

	// 行列作成
	Math::Matrix rot	= Math::Matrix::CreateRotationY(m_yaw);
	Math::Matrix trans	= Math::Matrix::CreateTranslation(m_mWorld.Translation() + m_moveDir * kMoveSpeed * deltaTime);
	m_mWorld			= rot * trans;
}

//===========================================================
// 描画関数
//===========================================================
void Player::DrawLit()
{
	if (!m_spModel)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel, m_mWorld
	);
}

//===========================================================
// Yaw 更新関数
//===========================================================
void Player::UpdateYaw(float deltaTime)
{
	// 移動していないときは向きを変えない
	if (m_moveDir.LengthSquared() <= 0.0001f) return;

	// 移動方向から目標 Yaw を求める
	const float		targetYaw = atan2f(m_moveDir.x, m_moveDir.z);

	// 現在の Yaw から目標 Yaw までの角度差を求め、最短経路（-π ～ +π）に正規化
	const float		angleDiff = DirectX::XMScalarModAngle(targetYaw - m_yaw);

	// 1秒間に回転できる最大角度（例えば 360.0f度）
	constexpr float turnSpeed = DirectX::XMConvertToRadians(360.0f);
	const float		maxRotate = turnSpeed * deltaTime;

	// std::clamp で1フレームの回転量を制限し、現在の Yaw に加算
	m_yaw += std::clamp(angleDiff, -maxRotate, maxRotate);
}
