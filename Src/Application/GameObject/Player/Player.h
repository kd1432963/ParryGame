#pragma once

class AnimationPlayer;
class ParrySystem;
class GroundPhysics;

template<class TStateId>
class StateMachine;

enum class PlayerStateId;
enum class ParryResult;

struct AttackInfo;

//===========================================================
// プレイヤーキャラクター
//===========================================================
class Player : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	Player();
	~Player()		override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Init()		override;
	void Update()	override;
	void DrawLit()	override;

	//===========================================================
	// 公開関数群
	//===========================================================
	void SetMoveDirection	(const Math::Vector3& dir)	{ m_moveDir = dir; }
	void SetFacingDirection(const Math::Vector3& dir)	{ m_facingDirection = dir; }
	void SetDashInput		(bool isDashPressed)		{ m_isDashPressed = isDashPressed; }
	void RequestParry();
	void OnHit(const AttackInfo& info);
	bool IsDead() const { return m_hp <= 0; }
	float GetBodyRadius() const { return kBodyRadius; }
	float GetBodyHeight() const { return kBodyHeight; }

	// パリィ成功時に実行する演出通知を設定する
	void SetParrySuccessCallback(
		std::function<void(
			const Math::Vector3&,
			ParryResult,
			const std::shared_ptr<KdGameObject>&)> callback
	)
	{
		m_parrySuccessCallback = std::move(callback);
	}

	// 通常ダメージを受けたときに実行する演出通知を設定する
	void SetDamageHitCallback(
		std::function<void(const Math::Vector3&)> callback
	)
	{
		m_damageHitCallback = std::move(callback);
	}

private:

	//===========================================================
	// 内部関数群
	//===========================================================
	// ステートマシンの状態登録
	void SetupStateMachine();

	// Normal
	void EnterNormal();
	void UpdateNormal(float deltaTime);
	
	// Parry
	void EnterParry();
	void UpdateParry(float deltaTime);
	void ExitParry();

	// Damage
	void EnterDamage();
	void UpdateDamage(float deltaTime);
	
	// Dead
	void EnterDead();

	void UpdateYaw(float deltaTime);
	void UpdateWorldMatrix(const Math::Vector3& position);
	void PlayLocomotionAnimation();
	void TakeDamage(int damage);

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::unique_ptr<StateMachine<PlayerStateId>>m_upStateMachine	= nullptr;
	std::unique_ptr<AnimationPlayer>			m_upAnimationPlayer = nullptr;
	std::unique_ptr<ParrySystem>				m_upParrySystem		= nullptr;
	std::unique_ptr<GroundPhysics>				m_upGroundPhysics	= nullptr;
	std::shared_ptr<KdModelWork>				m_spModel			= nullptr;

	std::function<void(const Math::Vector3&, 
		ParryResult,
		const std::shared_ptr<KdGameObject>&)>	m_parrySuccessCallback;
	std::function<void(const Math::Vector3&)>	m_damageHitCallback;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector3	m_moveDir			= Math::Vector3::Zero;
	Math::Vector3	m_facingDirection	= Math::Vector3::Zero;
	float			m_yaw				= 0.0f;
	float			m_modelScale		= 1.0f;
	bool			m_isDashPressed		= false;
	bool			m_isDashing			= false;
	int				m_hp				= kMaxHp;
	Math::Vector3	m_damageDirection	= Math::Vector3::Zero;
	float			m_damageTimer		= 0.0f;

	//===========================================================
	// 定数群
	//===========================================================
	static constexpr float	kMoveSpeed						= 5.0f;	// 移動速度
	static constexpr float	kWalkSpeed						= 5.0f;	// 歩き速度
	static constexpr float	kDashSpeed						= 8.0f; // ダッシュ速度
	static constexpr float	kWalkAnimationReferenceSpeed	= 3.5f; // 歩きアニメーションの基準速度
	static constexpr float	kDashAnimationReferenceSpeed	= 6.0f; // ダッシュアニメーションの基準速度
	static constexpr float	kStrafeAnimationReferenceSpeed	= 4.0f; // 横移動アニメーションの基準速度
	static constexpr float	kBackwardAnimationReferenceSpeed= 4.0f; // 後ろ移動アニメーションの基準速度
	static constexpr float	kLocomotionBlendTime			= 0.15f;// 移動アニメーション切り替え時の補間時間
	static constexpr float	kParryAnimationDuration			= 0.55f;// パリィアニメーションの再生時間
	static constexpr float	kActionBlendTime				= 0.06f;// アクションアニメーション切り替え時の補間時間
	static constexpr float	kFootOffsetY					= 0.0f; // 足元のオフセット（地面判定用）

	static constexpr int	kMaxHp			= 30;
	static constexpr float	kBodyCenterY	= 0.9f;
	static constexpr float	kBodyRadius		= 0.55f;
	static constexpr float	kBodyHeight		= 1.8f;

	static constexpr float	kDamageDuration			= 0.25f;
	static constexpr float	kDamageKnockbackSpeed	= 5.0f;
	static constexpr float	kDeathAnimationDuration = 0.85f;
};