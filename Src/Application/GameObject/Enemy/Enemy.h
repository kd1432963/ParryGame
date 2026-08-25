#pragma once

#include "../../Combat/IParryable.h"
#include "../../Combat/ILockOnTarget.h"

class Player;
class GroundPhysics;
class AnimationPlayer;

struct EnemyConfig;
enum class EnemyStateId;
enum class EnemyType;

template<class TStateId>
class StateMachine;

//===========================================================
// クラスの役割
//===========================================================
class Enemy : public KdGameObject , public IParryable , public ILockOnTarget
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	Enemy(EnemyType type);
	~Enemy()		override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Init()		override;
	void Update()	override;
	void DrawLit()	override;

	//===========================================================
	// 公開関数
	//===========================================================
	ParryResult OnParried()	override;

	void SetPlayer(const std::shared_ptr<Player>& player){ m_wpPlayer = player; }
	bool IsDead() const{ return m_parryDurability <= 0; }
	bool CanLockOn() const override{ return !IsDead();}
	Math::Vector3 GetLockOnPosition() const override;

private:

	//===========================================================
	// 内部処理
	//===========================================================
	// ステートマシンの状態登録
	void SetupStateMachine();

	// Idle
	void EnterIdle();
	void UpdateIdle(float deltaTime);

	// Chase
	void EnterChase();
	void UpdateChase(float deltaTime);

	// Attack
	void EnterAttack();
	void UpdateAttack(float deltaTime);
	void ExitAttack();

	// Recovery
	void UpdateRecovery(float deltaTime);

	// Stun
	void EnterStun();
	void UpdateStun(float deltaTime);
	void ExitStun();

	// Dead
	void EnterDead();
	void UpdateDead(float deltaTime);

	// Enemy 本体の動作
	float UpdateMove(
		const std::shared_ptr<Player>&	player,
		float							distance,
		float							deltaTime
	);
	float UpdateDirection(const std::shared_ptr<Player>& player);
	void UpdateWorldMatrix(
		const Math::Vector3&	pos,
		float					effectScaleXZ	= 1.0f,
		float					effectScaleY	= 1.0f
	);
	void StartComboAttack();

	// 設定された攻撃方法に応じて、近接判定または共鳴弾を生成する
	void CreateAttack();

	// Playerへ当てる1回限りの近接攻撃判定を生成する
	void CreateAttackHitBox();

	// 胸部からPlayer方向へ進む共鳴弾を生成する
	void CreateResonanceProjectile();

	// 攻撃アニメーション中の発射・命中基準位置を取得する
	Math::Vector3 GetAttackPosition();

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::unique_ptr<StateMachine<EnemyStateId>>	m_upStateMachine	= nullptr;
	std::unique_ptr<AnimationPlayer>			m_upAnimationPlayer = nullptr;
	std::unique_ptr<GroundPhysics>				m_upGroundPhysics	= nullptr;
	std::shared_ptr<KdModelWork>				m_spModel			= nullptr;
	std::weak_ptr<Player>						m_wpPlayer;
	std::unique_ptr<EnemyConfig>				m_upConfig			= nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	float			m_yaw					= 0.0f;
	int				m_comboAttackIndex		= 0;
	float			m_currentAttackHitDelay	= 0.0f;
	float			m_attackTimer			= 0.0f;
	float			m_attackHitTimer		= 0.0f;
	bool			m_isAttacking			= false;
	float			m_stunTimer				= 0.0f;
	float			m_parryScaleTimer		= 0.0f;
	float			m_parryColorTimer		= 0.0f;
	Math::Vector3	m_knockbackVelocity		= Math::Vector3::Zero;
	float			m_deadElapsedTime		= 0.0f;
	float			m_dissolveProgress		= 0.0f;
	int				m_parryDurability		= 0;

	//===========================================================
	// 固定値
	//===========================================================
	static constexpr float	kNormalBlendTime		= 0.12f;
	static constexpr float	kAttackBlendTime		= 0.08f;
	static constexpr float	kReactionBlendTime		= 0.05f;
	static constexpr float	kParryStunTime			= 1.0f;
	static constexpr float	kParryScaleTime			= 0.20f;
	static constexpr float	kParryColorTime			= 0.10f;
	static constexpr float	kParryKnockbackPower	= 7.0f;
	static constexpr float	kKnockbackDeceleration	= 15.0f;

	// 死亡関連の定数
	static constexpr float	kDeathBlendTime				= 0.10f;
	static constexpr float	kDeathKnockbackPower		= 10.0f;
	static constexpr float	kDeathKnockbackDeceleration = 18.0f;
	static constexpr float	kDeathScaleTime				= 0.25f;
	static constexpr float	kDeathFlashTime				= 0.12f;
	static constexpr float	kDissolveDelay				= 0.70f;
	static constexpr float	kDissolveDuration			= 0.80f;
	static constexpr float	kDissolveEdgeRange			= 0.05f;
};