#include "Player.h"

#include "../../main.h"
#include "PlayerState.h"
#include "../../StateMachine/StateMachine.h"
#include "../../Animation/AnimationPlayer.h"
#include "../../Combat/ParrySystem.h"
#include "../Physics/GroundPhysics.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
Player::Player()
	:	m_upStateMachine(std::make_unique<StateMachine<PlayerStateId>>()),
		m_upAnimationPlayer(std::make_unique<AnimationPlayer>()),
		m_upParrySystem(std::make_unique<ParrySystem>()),
		m_upGroundPhysics(std::make_unique<GroundPhysics>())
{
}
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

#ifdef _DEBUG
	if (!m_pDebugWire)
	{
		m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	}
#endif

	// 原点に配置
	SetPos({ 0.0f, 0.0f, 0.0f });
	UpdateWorldMatrix(GetPos());

	// ステートマシンの状態登録
	SetupStateMachine();
}

//===========================================================
// 更新関数
//===========================================================
void Player::Update()
{
	const float deltaTime = Application::Instance().GetDeltaTime();

	// 現在状態の Update だけを実行する
	if (m_upStateMachine)
	{
		m_upStateMachine->Update(deltaTime);
	}

	// アニメーションを更新する
	if (m_upAnimationPlayer && m_spModel)
	{
		m_upAnimationPlayer->Update(
			*m_spModel,
			deltaTime
		);
	}

	// 行動状態にかかわらず、最後に地面へ接地させる
	if (m_upGroundPhysics)
	{
		m_upGroundPhysics->Update(
			*this,
			deltaTime,
			kFootOffsetY
		);
	}
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
// パリィ要求関数
//===========================================================
void Player::RequestParry()
{
	if (!m_upStateMachine || !m_upParrySystem)						return;
	if (!m_upStateMachine->IsCurrentState(PlayerStateId::Normal))	return;
	if (m_upParrySystem->IsBusy())									return;

	m_upStateMachine->ChangeState(PlayerStateId::Parry);
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

//===========================================================
// ワールド行列更新関数
//===========================================================
void Player::UpdateWorldMatrix(const Math::Vector3& position)
{
	const Math::Matrix	scale		= Math::Matrix::CreateScale(m_modelScale);
	const Math::Matrix	rotation	= Math::Matrix::CreateRotationY(m_yaw);
	const Math::Matrix	translation = Math::Matrix::CreateTranslation(position);

	m_mWorld = scale * rotation * translation;
}

//===========================================================
// 移動アニメーション再生関数
//===========================================================
void Player::PlayLocomotionAnimation()
{
	if (!m_upAnimationPlayer || !m_spModel) return;

	const bool	isMoving				= m_moveDir.LengthSquared() > 0.01f;
	std::string	animationName			= "PlayerBlindEcho_Idle";
	float		animationReferenceSpeed = 1.0f;

	if (isMoving)
	{
		animationName			= "PlayerBlindEcho_Run";
		animationReferenceSpeed = kRunAnimationReferenceSpeed;
	}

	const float	moveSpeed		= m_isDashing ? kDashSpeed : kWalkSpeed;
	const float	playbackSpeed	= isMoving ? moveSpeed / animationReferenceSpeed : 1.0f;

	m_upAnimationPlayer->Play(
		*m_spModel,
		animationName,
		true,
		0.0f,
		kLocomotionBlendTime
	);

	// 座標の移動速度を変えても、足運びが同じ割合で追従する
	m_upAnimationPlayer->SetAnimationSpeed(playbackSpeed);
}
