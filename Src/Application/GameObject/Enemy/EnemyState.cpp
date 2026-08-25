#include "EnemyState.h"

#include "Enemy.h"
#include "../Player/Player.h"
#include "EnemyConfig.h"
#include "../../StateMachine/StateMachine.h"
#include "../../Animation/AnimationPlayer.h"

namespace
{
	// ノックバック速度の向きを保ったまま、指定した減速度で0へ近づける
	void DecelerateKnockback(
		Math::Vector3& velocity,
		float deceleration,
		float deltaTime
	)
	{
		float speed = velocity.Length();

		if (speed <= 0.0f) return;

		speed = std::max(0.0f, speed - deceleration * deltaTime);

		if (speed <= 0.0f)
		{
			velocity = Math::Vector3::Zero;
			return;
		}

		velocity.Normalize();
		velocity *= speed;
	}
}

//===========================================================
// ステートマシンの状態登録
//===========================================================
void Enemy::SetupStateMachine()
{
	//===========================================================
	// 通常状態
	//===========================================================
	m_upStateMachine->RegisterState(
		EnemyStateId::Idle,
		[this]() { EnterIdle(); },
		nullptr,
		nullptr
	);
	//===========================================================
	// 追跡状態
	//===========================================================
	m_upStateMachine->RegisterState(
		EnemyStateId::Chase,
		[this]() { EnterChase(); },
		[this](float deltaTime) { UpdateChase(deltaTime); },
		nullptr
	);
	//===========================================================
	// 攻撃状態
	//===========================================================
	m_upStateMachine->RegisterState(
		EnemyStateId::Attack,
		[this]() { EnterAttack(); },
		[this](float deltaTime) { UpdateAttack(deltaTime); },
		[this]() { ExitAttack(); }
	);
	//===========================================================
	// 硬直状態
	//===========================================================
	m_upStateMachine->RegisterState(
		EnemyStateId::Recovery,
		nullptr,
		[this](float deltaTime) { UpdateRecovery(deltaTime); },
		nullptr
	);
	//===========================================================
	// パリィ硬直状態
	//===========================================================
	m_upStateMachine->RegisterState(
		EnemyStateId::Stun,
		[this]() { EnterStun(); },
		[this](float deltaTime) { UpdateStun(deltaTime); },
		[this]() { ExitStun(); }
	);
	//===========================================================
	// 死亡状態
	//===========================================================
	m_upStateMachine->RegisterState(
		EnemyStateId::Dead,
		[this]() { EnterDead(); },
		[this](float deltaTime) { UpdateDead(deltaTime); },
		nullptr
	);

	m_upStateMachine->Start(EnemyStateId::Chase);
}

//===========================================================
// Idle
//===========================================================
void Enemy::EnterIdle()
{
	m_upAnimationPlayer->Play(
		*m_spModel,
		m_upConfig->idleAnimationName,
		true,
		0.0f,
		kNormalBlendTime
	);
}

//===========================================================
// Chase
//===========================================================
void Enemy::EnterChase()
{
	m_upAnimationPlayer->Play(
		*m_spModel,
		m_upConfig->chaseAnimationName,
		true,
		m_upConfig->walkAnimationCycleTime,
		kNormalBlendTime
	);
}

void Enemy::UpdateChase(float deltaTime)
{
	// プレイヤーがいない or 死亡している場合は追跡しない
	const auto player = m_wpPlayer.lock();

	if (!player)return;
	//if (!player || player->IsDead()) return;

	// プレイヤーの方向を向く、距離が 0 以下なら追跡しない
	float distance = UpdateDirection(player);
	if (distance <= 0.0f) return;

	// 移動する
	distance = UpdateMove(
		player,
		distance,
		deltaTime
	);

	// 攻撃範囲内に入ったら攻撃状態に遷移する
	if (distance >= m_upConfig->minAttackRange &&
		distance <= m_upConfig->maxAttackRange)
	{
		m_upStateMachine->ChangeState(EnemyStateId::Attack);
	}
}

//===========================================================
// Attack
//===========================================================
void Enemy::EnterAttack()
{
	m_comboAttackIndex = 0;

	StartComboAttack();
}

void Enemy::UpdateAttack(float deltaTime)
{
	const auto player = m_wpPlayer.lock();

	if (!player || player->IsDead()) return;

	UpdateDirection(player);

	m_attackTimer = std::max(0.0f, m_attackTimer - deltaTime);
	m_attackHitTimer += deltaTime;

	// 設定時刻を越えた瞬間に、攻撃判定を一度だけ生成する
	if (!m_hasCreatedAttack &&
		m_attackHitTimer >= m_currentAttackHitDelay)
	{
		CreateAttack();
		m_hasCreatedAttack = true;

		const bool hasNextAttack =
			m_comboAttackIndex + 1 <
			m_upConfig->attackStepCount;

		// 最後の一撃後は、残りのモーションをRecoveryで再生する
		if (!hasNextAttack)
		{
			m_upStateMachine->ChangeState(EnemyStateId::Recovery);
			return;
		}
	}

	// 攻撃モーションの次の攻撃タイミングを過ぎたら、次の攻撃へ移行する
	const EnemyAttackStepConfig& stepConfig = m_upConfig->attackSteps[m_comboAttackIndex];
	const float nextAttackTime				= stepConfig.durationSeconds * stepConfig.nextAttackTimingRate;
	if (!m_hasCreatedAttack || m_attackHitTimer < nextAttackTime) return;

	++m_comboAttackIndex;
	StartComboAttack();
}

void Enemy::ExitAttack()
{
	m_hasCreatedAttack = false;
	m_attackHitTimer	= 0.0f;
}

//===========================================================
// Recovery
//===========================================================
void Enemy::UpdateRecovery(float deltaTime)
{
	const auto player = m_wpPlayer.lock();

	if (!player || player->IsDead()) return;

	// 攻撃後の隙では移動せず、Player方向だけを向く
	UpdateDirection(player);

	m_attackTimer = std::max(0.0f, m_attackTimer - deltaTime);

	if (m_attackTimer > 0.0f) return;

	m_upStateMachine->ChangeState(EnemyStateId::Chase);
}

//===========================================================
// Stun
//===========================================================
void Enemy::EnterStun()
{
	m_upAnimationPlayer->Play(
		*m_spModel,
		m_upConfig->hitAnimationName,
		false,
		kParryStunTime,
		kReactionBlendTime
	);

	// 攻撃途中でも確実に中断する
	m_hasCreatedAttack	= false;
	m_attackTimer		= 0.0f;
	m_attackHitTimer	= 0.0f;

	m_stunTimer			= kParryStunTime;
	m_parryScaleTimer	= kParryScaleTime;
	m_parryColorTimer	= kParryColorTime;
	m_knockbackVelocity = Math::Vector3::Zero;

	const auto player	= m_wpPlayer.lock();

	if (!player) return;

	Math::Vector3 knockDirection = GetPos() - player->GetPos();
	knockDirection.y = 0.0f;

	if (knockDirection.LengthSquared() <= 0.0f) return;

	knockDirection.Normalize();
	m_knockbackVelocity = knockDirection * kParryKnockbackPower;
}

void Enemy::UpdateStun(float deltaTime)
{
	m_stunTimer = std::max(0.0f, m_stunTimer - deltaTime);

	// Playerと反対方向へ動かし、速度を徐々に0へ近づける
	Math::Vector3 position = GetPos();
	position += m_knockbackVelocity * deltaTime;

	DecelerateKnockback(
		m_knockbackVelocity,
		kKnockbackDeceleration,
		deltaTime
	);

	// パリィ直後だけ横へ広げ、縦へ潰す
	float effectScaleXZ = 1.0f;
	float effectScaleY	= 1.0f;

	// パリィ直後の拡大縮小アニメーション
	if (m_parryScaleTimer > 0.0f)
	{
		m_parryScaleTimer = std::max(0.0f,m_parryScaleTimer - deltaTime);

		const float scaleRate = m_parryScaleTimer / kParryScaleTime;
		effectScaleXZ	= 1.0f + scaleRate * 0.30f;
		effectScaleY	= 1.0f - scaleRate * 0.30f;
	}

	m_parryColorTimer = std::max(0.0f, m_parryColorTimer - deltaTime);

	// 行列更新
	UpdateWorldMatrix(position, effectScaleXZ, effectScaleY);

	// スタンが続いてる場合は早期 return
	if (m_stunTimer > 0.0f) return;

	// スタンが終わったら追跡状態へ遷移する
	m_upStateMachine->ChangeState(EnemyStateId::Chase);
}

void Enemy::ExitStun()
{
	// 値をリセット
	m_stunTimer			= 0.0f;
	m_parryScaleTimer	= 0.0f;
	m_parryColorTimer	= 0.0f;
	m_knockbackVelocity = Math::Vector3::Zero;

	// 行列を元に戻す
	UpdateWorldMatrix(GetPos());
}

//===========================================================
// Dead
//===========================================================
void Enemy::EnterDead()
{
	// 撃破アニメーションを再生する
	m_upAnimationPlayer->Play(
		*m_spModel,
		m_upConfig->deathAnimationName,
		false,
		m_upConfig->deathAnimationTime,
		kDeathBlendTime
	);

	// 撃破時の値をリセットする
	m_hasCreatedAttack	= false;
	m_attackTimer		= 0.0f;
	m_attackHitTimer	= 0.0f;
	m_stunTimer			= 0.0f;
	m_parryScaleTimer	= kDeathScaleTime;
	m_parryColorTimer	= kDeathFlashTime;
	m_knockbackVelocity = Math::Vector3::Zero;
	m_deadElapsedTime	= 0.0f;
	m_dissolveProgress	= 0.0f;

	const auto player = m_wpPlayer.lock();

	if (player)
	{
		Math::Vector3 knockDirection = GetPos() - player->GetPos();
		knockDirection.y = 0.0f;

		if (knockDirection.LengthSquared() > 0.0f)
		{
			knockDirection.Normalize();
			m_knockbackVelocity = knockDirection * kDeathKnockbackPower;
		}
	}

	UpdateWorldMatrix(GetPos());

	// 撃破位置だけを外へ通知し、演出内容は呼び出し側へ任せる
	//	if (m_defeatedCallback)
	{
	//	m_defeatedCallback(GetPos());
	}
}

void Enemy::UpdateDead(float deltaTime)
{
	m_deadElapsedTime += deltaTime;

	// 通常のStunより強く後退させ、短時間で停止させる
	Math::Vector3 position = GetPos();
	position += m_knockbackVelocity * deltaTime;

	DecelerateKnockback(
		m_knockbackVelocity,
		kDeathKnockbackDeceleration,
		deltaTime
	);

	// 撃破直後は通常パリィより強く横へ広げる
	float effectScaleXZ = 1.0f;
	float effectScaleY	= 1.0f;

	if (m_parryScaleTimer > 0.0f)
	{
		m_parryScaleTimer = std::max(
			0.0f,
			m_parryScaleTimer - deltaTime
		);

		const float scaleRate = m_parryScaleTimer / kDeathScaleTime;
		effectScaleXZ = 1.0f + scaleRate * 0.40f;
		effectScaleY = 1.0f - scaleRate * 0.40f;
	}

	m_parryColorTimer = std::max(0.0f, m_parryColorTimer - deltaTime);

	UpdateWorldMatrix(position, effectScaleXZ, effectScaleY);

	// 撃破の手応えを見せてからディゾルブを始める
	if (m_deadElapsedTime <= kDissolveDelay) return;

	m_dissolveProgress = std::clamp(
		(m_deadElapsedTime - kDissolveDelay) / kDissolveDuration,
		0.0f,
		1.0f
	);

	// モーションとディゾルブの両方が終わってから削除する
	if (!m_upAnimationPlayer->IsAnimationEnd() ||
		m_dissolveProgress < 1.0f)
	{
		return;
	}

	m_isExpired = true;
}