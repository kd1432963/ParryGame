#include "PlayerState.h"

#include "Player.h"
#include "../../StateMachine/StateMachine.h"
#include "../../Combat/ParrySystem.h"
#include "../../Animation/AnimationPlayer.h"

//===========================================================
// ステートマシンの状態登録
//===========================================================
void Player::SetupStateMachine()
{
	//===========================================================
	// 通常状態
	//===========================================================
	m_upStateMachine->RegisterState(
		PlayerStateId::Normal,
		[this]() { EnterNormal(); },
		[this](float deltaTime) { UpdateNormal(deltaTime); },
		nullptr
	);
	//===========================================================
	// パリィ状態
	//===========================================================
	m_upStateMachine->RegisterState(
		PlayerStateId::Parry,
		[this]() { EnterParry(); },
		[this](float deltaTime) { UpdateParry(deltaTime); },
		[this]() { ExitParry(); }
	);
	//===========================================================
	// 被弾状態
	//===========================================================
	m_upStateMachine->RegisterState(
		PlayerStateId::Damage,
		[this]() { EnterDamage(); },
		[this](float deltaTime) { UpdateDamage(deltaTime); },
		nullptr
	);
	//===========================================================
	// 死亡状態
	//===========================================================
	m_upStateMachine->RegisterState(
		PlayerStateId::Dead,
		[this]() { EnterDead(); },
		nullptr,
		nullptr
	);

	m_upStateMachine->Start(PlayerStateId::Normal);
}

//===========================================================
// 通常状態
//===========================================================
void Player::EnterNormal()
{
	// 移動アニメーションを再生する
	PlayLocomotionAnimation();
}

void Player::UpdateNormal(float deltaTime)
{
	const bool		isMoving = m_moveDir.LengthSquared() > 0.01f;
	m_isDashing = m_isDashPressed && isMoving;

	Math::Vector3	position	= GetPos();
	const float		moveSpeed	= m_isDashing ? kDashSpeed : kWalkSpeed;

	// 入力方向へ移動する
	position += m_moveDir * moveSpeed * deltaTime;

	// 移動中だけ進行方向経向きを変える
	UpdateYaw(deltaTime);

	// ワールド行列を更新する
	UpdateWorldMatrix(position);

	// 移動アニメーションを再生する
	PlayLocomotionAnimation();
}

//===========================================================
// パリィ状態
//===========================================================
void Player::EnterParry()
{
	// パリィ中は移動を止め、受付と専用モーションを開始する
	m_moveDir	= Math::Vector3::Zero;
	m_isDashing = false;

	if (m_upParrySystem)
	{
		m_upParrySystem->Start();
	}

	if (m_upAnimationPlayer && m_spModel)
	{
		m_upAnimationPlayer->Play(
			*m_spModel,
			"PlayerBlindEcho_Parry",
			false,
			kParryAnimationDuration,
			kActionBlendTime
		);
	}

#ifdef _DEBUG
	KdDebugGUI::Instance().AddLog("Parry Start\n");
#endif
}

void Player::UpdateParry(float deltaTime)
{
	// パリィシステムが存在しない場合は通常状態に戻す
	if (!m_upParrySystem)
	{
		m_upStateMachine->ChangeState(PlayerStateId::Normal);
		return;
	}

	// パリィシステムを更新する
	m_upParrySystem->Update(deltaTime);

	// パリィシステムがビジー状態でない場合は通常状態に戻す
	if (m_upParrySystem->IsBusy()) return;
	m_upStateMachine->ChangeState(PlayerStateId::Normal);
}

void Player::ExitParry()
{
	if (!m_upParrySystem) return;

	m_upParrySystem->Cancel();
}

//===========================================================
// Damage
//===========================================================
void Player::EnterDamage()
{
	m_moveDir		= Math::Vector3::Zero;
	m_damageTimer	= 0.0f;
	m_isDashing		= false;

	if (m_upAnimationPlayer && m_spModel)
	{
		// 被弾硬直と同じ時間でモーションを最後まで再生する
		m_upAnimationPlayer->Play(
			*m_spModel,
			"PlayerBlindEcho_Hit",
			false,
			kDamageDuration,
			kActionBlendTime
		);
	}
}

void Player::UpdateDamage(float deltaTime)
{
	m_damageTimer += deltaTime;

	const float remainRate = 1.0f - std::clamp(
		m_damageTimer / kDamageDuration,
		0.0f,
		1.0f
	);

	Math::Vector3 position = GetPos();

	position += m_damageDirection *
		kDamageKnockbackSpeed *
		remainRate *
		deltaTime;

	UpdateWorldMatrix(position);

	if (m_damageTimer < kDamageDuration) return;

	m_upStateMachine->ChangeState(PlayerStateId::Normal);
}

//===========================================================
// Dead
//===========================================================
void Player::EnterDead()
{
	// 死亡後は入力が残っていても移動しない
	m_moveDir	= Math::Vector3::Zero;
	m_isDashing = false;

	if (m_upAnimationPlayer && m_spModel)
	{
		m_upAnimationPlayer->Play(
			*m_spModel,
			"PlayerBlindEcho_Death",
			false,
			kDeathAnimationDuration,
			kActionBlendTime
		);
	}
}