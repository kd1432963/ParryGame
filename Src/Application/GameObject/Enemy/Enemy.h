#pragma once

class Player;
class GroundPhysics;
class AnimationPlayer;

//===========================================================
// クラスの役割
//===========================================================
class Enemy : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	Enemy();
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
	void SetPlayer(const std::shared_ptr<Player>& player){ m_wpPlayer = player; }

private:

	//===========================================================
	// 内部処理
	//===========================================================
	void UpdateWorldMatrix(
		const Math::Vector3&	pos,
		float					effectScaleXZ	= 1.0f,
		float					effectScaleY	= 1.0f
	);

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	//std::unique_ptr<StateMachine<EnemyStateId>>m_upStateMachine	= nullptr;
	std::unique_ptr<AnimationPlayer>			m_upAnimationPlayer = nullptr;
	std::unique_ptr<GroundPhysics>				m_upGroundPhysics	= nullptr;
	std::shared_ptr<KdModelWork>				m_spModel			= nullptr;
	std::weak_ptr<Player>						m_wpPlayer;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector3	m_moveDir		= Math::Vector3::Zero;
	float			m_yaw			= 0.0f;
	float			m_modelScale	= 2.0f;
	float			m_moveSpeed		= 0.0f;

	//===========================================================
	// 固定値
	//===========================================================
};