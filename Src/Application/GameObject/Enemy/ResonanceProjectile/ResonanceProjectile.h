#pragma once

class Enemy;

//===========================================================
// 共鳴射手が発射する低速弾の移動・衝突・描画を管理する
//===========================================================
class ResonanceProjectile : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	ResonanceProjectile(
		const Math::Vector3&			startPosition,
		const Math::Vector3&			direction,
		float							speed,
		float							hitRadius,
		float							lifeTime,
		int								damage,
		const std::shared_ptr<Enemy>&	owner
	);
	~ResonanceProjectile() override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Init()			override;
	void Update()		override;
	void DrawEffect()	override;

private:

	//===========================================================
	// 内部処理
	//===========================================================

	// 現在位置に球形判定を作り、Player へ当たったか調べる
	bool TryHitPlayer();

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::weak_ptr<Enemy>				m_wpOwner;
	std::shared_ptr<KdSquarePolygon>	m_spBillboard = nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector3	m_position		= Math::Vector3::Zero;
	Math::Vector3	m_direction		= Math::Vector3::Forward;
	float			m_speed			= 0.0f;
	float			m_hitRadius		= 0.0f;
	float			m_lifeTime		= 0.0f;
	float			m_elapsedTime	= 0.0f;
	int				m_damage		= 0;
};