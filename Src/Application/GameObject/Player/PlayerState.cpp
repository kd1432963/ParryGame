#include "PlayerState.h"

#include "Player.h"
#include "../../StateMachine/StateMachine.h"

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
		[this]() { /* Enter処理 */ },
		[this](float deltaTime) { /* Update処理 */ },
		[this]() { /* Exit処理 */ }
	);
	//===========================================================
	// 死亡状態
	//===========================================================
	m_upStateMachine->RegisterState(
		PlayerStateId::Dead,
		[this]() { /* Enter処理 */ },
		[this](float deltaTime) { /* Update処理 */ },
		[this]() { /* Exit処理 */ }
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
	// パリィ状態に入ったときの処理
}

void Player::UpdateParry(float deltaTime)
{
	// パリィ状態の更新処理
}

void Player::ExitParry()
{
	// パリィ状態から抜けるときの処理
}