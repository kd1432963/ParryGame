#pragma once

class CameraManager;
class ILockOnTarget;

//===========================================================
// ロックオン中の敵位置へ四隅の照準を表示する
//===========================================================
class LockOnMarker : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	LockOnMarker() = default;
	~LockOnMarker()	override = default;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Update()		override;
	void DrawSprite()	override;

	//===========================================================
	// 公開関数群
	//===========================================================
	void SetCameraManager(
		const std::shared_ptr<CameraManager>& cameraManager
	)
	{
		m_wpCameraManager = cameraManager;
	}

private:

	//===========================================================
	// 内部関数群
	//===========================================================
	void Hide();

	//===========================================================
	// 内部変数群
	//===========================================================
	std::weak_ptr<CameraManager>	m_wpCameraManager;
	std::weak_ptr<ILockOnTarget>	m_wpDisplayedTarget;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector2	m_screenPosition	= Math::Vector2::Zero;
	float			m_appearRate		= 0.0f;
	float			m_visibleTime		= 0.0f;
	bool			m_isVisible			= false;
};