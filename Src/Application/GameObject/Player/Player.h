#pragma once

class AnimationPlayer;

template<class TStateId>
class StateMachine;

enum class PlayerStateId;

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
	void SetDashInput		(bool isDashPressed)		{ m_isDashPressed = isDashPressed; }

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

	void UpdateYaw(float deltaTime);
	void UpdateWorldMatrix(const Math::Vector3& position);
	void PlayLocomotionAnimation();

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::unique_ptr<StateMachine<PlayerStateId>>m_upStateMachine	= nullptr;
	std::unique_ptr<AnimationPlayer>			m_upAnimationPlayer = nullptr;
	std::shared_ptr<KdModelWork>				m_spModel			= nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector3	m_moveDir		= Math::Vector3::Zero;
	float			m_yaw			= 0.0f;
	float			m_modelScale	= 1.0f;
	bool			m_isDashPressed	= false;
	bool			m_isDashing		= false;

	//===========================================================
	// 定数群
	//===========================================================
	static constexpr float	kMoveSpeed						= 5.0f;	// 移動速度
	static constexpr float	kWalkSpeed						= 5.0f;
	static constexpr float	kDashSpeed						= 8.0f;
	static constexpr float	kRunAnimationReferenceSpeed		= 5.0f;
	static constexpr float	kLocomotionBlendTime			= 0.15f;// 移動アニメーション切り替え時の補間時間
};