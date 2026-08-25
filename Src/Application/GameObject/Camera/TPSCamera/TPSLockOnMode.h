#pragma once

class ILockOnTarget;

//===========================================================
// ロックオン中の注視点・距離・カメラ方向を計算する
//===========================================================
class TPSLockOnMode
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	TPSLockOnMode()		= default;
	~TPSLockOnMode()	= default;

	//===========================================================
	// 公開関数群
	//===========================================================
	// 有効な対象をロックオン先として保存する
	bool SetTarget(const std::shared_ptr<ILockOnTarget>& target);
	void ClearTarget();
	bool HasTarget() const;

	std::shared_ptr<ILockOnTarget> GetTarget() const;

	// ロックオン時に必要なカメラ情報を計算する
	bool Update(
		const Math::Vector3&	followPosition,
		float					baseDistance,
		float					focusHeight,
		float					unscaledDeltaTime,
		Math::Vector3&			cameraAngle,
		Math::Vector3&			outFocusPosition,
		float&					outDistance
	) const;

private:

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::weak_ptr<ILockOnTarget>	m_wpTarget;

	//===========================================================
	// 状態値
	//===========================================================
	float	m_maxDistance			= 8.0f;	// 計算後のカメラ距離の上限
	float	m_distancePerSeparation = 0.35f;// プレイヤーと敵の距離が 1m 離れるごとに、カメラはこの値だけ遠ざかる
	float	m_focusBias				= 0.40f;//	プレイヤーと敵の中間点よりも、敵側へ注視点を寄せる割合
	float	m_rotationSharpness		= 8.0f;	// カメラの回転を滑らかにするための補間係数
};