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
		[this]() { /* Enter処理 */ },
		[this](float deltaTime) { /* Update処理 */ },
		[this]() { /* Exit処理 */ }
	);
	//===========================================================
	// パリィ状態
	//===========================================================
	m_upStateMachine->RegisterState(
		PlayerStateId::Parry,
		[this]() { /* Enter処理 */ },
		[this](float deltaTime) { /* Update処理 */ },
		[this]() { /* Exit処理 */ }
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