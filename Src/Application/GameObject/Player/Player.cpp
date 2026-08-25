#include "Player.h"

#include "../../main.h"
#include "PlayerState.h"
#include "../Physics/GroundPhysics.h"
#include "../../StateMachine/StateMachine.h"
#include "../../Animation/AnimationPlayer.h"
#include "../../Combat/ParrySystem.h"
#include "../../Combat/AttackInfo.h"
#include "../../Combat/IParryable.h"

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
				"Asset/Models/Player/PlayerBlindEcho/PlayerBlindEcho.gltf")
		);
	}

	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
		m_pCollider->RegisterCollisionShape(
			"Player",
			Math::Vector3(0.0f, kBodyCenterY, 0.0f),
			kBodyRadius,
			KdCollider::TypeDamage
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
	m_upStateMachine->Update(deltaTime);

	// アニメーションを更新する
	if (m_spModel)
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
// HitBox に当たった際の処理関数
//===========================================================
void Player::OnHit(const AttackInfo& info)
{
	if (IsDead()) return;

	const auto	attacker = info.attacker.lock();
	const auto	parryable = std::dynamic_pointer_cast<IParryable>(attacker);

	// 受付中かつ、攻撃者がパリィ可能なら成功
	if (m_upParrySystem->IsActive() && parryable)
	{
		m_upParrySystem->Success();

		const ParryResult parryResult = parryable->OnParried();

		// 成功時は即座に通常状態にして、パリィ入力を受け付けられるようにする
		m_upStateMachine->ChangeState(PlayerStateId::Normal);

		// パリィ発生地点・結果・パリィされた敵を通知する
		if (m_parrySuccessCallback)
		{
			m_parrySuccessCallback(
				info.hitPos,
				parryResult,
				attacker
			);
		}

		// 成功時は即座に通常状態へ戻して、
		// 次のパリィ入力を受け付けられるようにする
		m_upStateMachine->ChangeState(PlayerStateId::Normal);

		return;
	}

	const int hpBeforeDamage = m_hp;

	TakeDamage(info.damage);

	if (m_hp >= hpBeforeDamage)return;

	// ダメージを受けた位置を通知する
	if (m_damageHitCallback)
	{
		m_damageHitCallback(info.hitPos);
	}

	if (IsDead())return;

	// ノックバック方向を計算する
	m_damageDirection = Math::Vector3::Zero;
	if (attacker)
	{
		m_damageDirection = GetPos() - attacker->GetPos();
		m_damageDirection.y = 0.0f;

		if (m_damageDirection.LengthSquared() > 0.0001f)
		{
			m_damageDirection.Normalize();
		}
	}

	// Damage 中に再び攻撃された場合も硬直時間を延長する
	m_damageTimer = 0.0f;

	m_upStateMachine->ChangeState(PlayerStateId::Damage);
}

//===========================================================
// ダメージ処理関数
//===========================================================
void Player::TakeDamage(int damage)
{
	if (damage <= 0) return;

	m_hp = std::max(0, m_hp - damage);

#ifdef _DEBUG
	KdDebugGUI::Instance().AddLog("Player HP = %d\n", m_hp);
#endif

	if (!IsDead()) return;

	m_upStateMachine->ChangeState(PlayerStateId::Dead);
}

//===========================================================
// Yaw更新関数
//===========================================================
void Player::UpdateYaw(float deltaTime)
{
	// 向く方向が指定されていない場合は移動方向を使用する
	Math::Vector3 facingDirection = m_moveDir;

	if (m_facingDirection.LengthSquared() > 0.0001f)
	{
		facingDirection = m_facingDirection;
	}

	if (facingDirection.LengthSquared() <= 0.0001f) return;

	// 向く方向から目標Yawを求める
	const float targetYaw = atan2f(
		-facingDirection.x,
		-facingDirection.z
	);

	// 現在角度から目標角度までの最短角度差を求める
	const float angleDifference = DirectX::XMScalarModAngle(targetYaw - m_yaw);

	// 1フレームで回転できる最大角度
	constexpr float	turnSpeed = DirectX::XMConvertToRadians(360.0f);
	const float		maxRotate = turnSpeed * deltaTime;

	m_yaw += std::clamp(angleDifference, -maxRotate, maxRotate);
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

	const bool	isMoving = m_moveDir.LengthSquared() > 0.0001f;
	const bool	isLockOn = m_facingDirection.LengthSquared() > 0.0001f;

	std::string	animationName = "PlayerBlindEcho_Idle";
	float		animationReferenceSpeed = 1.0f;

	// 通常時は前進またはダッシュを再生する
	if (isMoving)
	{
		animationName = m_isDashing ?
			"PlayerBlindEcho_Dash" :
			"PlayerBlindEcho_Walk";

		animationReferenceSpeed = m_isDashing ?
			kDashAnimationReferenceSpeed :
			kWalkAnimationReferenceSpeed;
	}

	// ロックオン中は敵を向いたまま、移動方向に合うモーションを選ぶ
	if (isMoving && isLockOn)
	{
		Math::Vector3 facingDirection = m_facingDirection;
		facingDirection.Normalize();

		// 敵方向を前、その直角方向を右とする
		const Math::Vector3 rightDirection =
		{
			facingDirection.z,
			0.0f,
			-facingDirection.x
		};

		const float	forwardAmount = m_moveDir.Dot(facingDirection);
		const float	rightAmount = m_moveDir.Dot(rightDirection);

		const float	absoluteForward = fabsf(forwardAmount);
		const float	absoluteRight = fabsf(rightAmount);

		// 45度付近でWalkとStrafeが交互に切り替わることを防ぐ
		constexpr float kDirectionSelectionBias = 0.15f;

		if (absoluteRight > absoluteForward + kDirectionSelectionBias)
		{
			animationName = rightAmount > 0.0f ?
				"PlayerBlindEcho_StrafeRight" :
				"PlayerBlindEcho_StrafeLeft";

			animationReferenceSpeed =
				kStrafeAnimationReferenceSpeed;
		}
		else if (forwardAmount < -kDirectionSelectionBias)
		{
			animationName =
				"PlayerBlindEcho_RunBackward";

			animationReferenceSpeed =
				kBackwardAnimationReferenceSpeed;
		}
	}

	const float	moveSpeed = m_isDashing ? kDashSpeed : kWalkSpeed;
	const float	playbackSpeed = isMoving ? moveSpeed / animationReferenceSpeed : 1.0f;

	m_upAnimationPlayer->Play(
		*m_spModel,
		animationName,
		true,
		0.0f,
		kLocomotionBlendTime
	);

	m_upAnimationPlayer->SetAnimationSpeed(playbackSpeed);
}
