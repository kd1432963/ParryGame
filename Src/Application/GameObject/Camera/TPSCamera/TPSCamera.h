#pragma once

#include "../CameraBase.h"
#include "TPSCameraMode.h"

class ILockOnTarget;
class LockOnTargetManager;
class TPSFreeLookMode;
class TPSLockOnMode;

//===========================================================
// プレイヤーを追従する三人称カメラ
//===========================================================
class TPSCamera : public CameraBase
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	TPSCamera();
	~TPSCamera()		override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Init()			override;
	void PostUpdate()	override;

	//===========================================================
	// 公開関数群
	//===========================================================
	void OnActivated()	override;

	bool SetLockOnTarget(const std::shared_ptr<ILockOnTarget>& target);
	void ClearLockOnTarget();
	bool HasLockOnTarget() const;

	void SetLockOnTargetManager(
		const std::shared_ptr<LockOnTargetManager>& targetManager
	)
	{
		m_wpLockOnTargetManager = targetManager;
	}

	std::shared_ptr<ILockOnTarget> GetLockOnTarget() const;

private:

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::unique_ptr<TPSFreeLookMode>	m_upFreeLookMode	= nullptr;
	std::unique_ptr<TPSLockOnMode>		m_upLockOnMode		= nullptr;
	std::weak_ptr<LockOnTargetManager>	m_wpLockOnTargetManager;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector3	m_focusPosition		= Math::Vector3::Zero;
	TPSCameraModeId	m_mode				= TPSCameraModeId::FreeLook;

	float			m_currentDistance	= 5.0f;
	float			m_baseDistance		= 5.0f;
	float			m_focusHeight		= 1.5f;
	float			m_focusSharpness	= 10.0f;
	float			m_distanceSharpness = 8.0f;

	bool			m_hasCameraState	= false;
};