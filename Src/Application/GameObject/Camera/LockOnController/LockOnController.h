#pragma once

class CameraManager;
class ILockOnTarget;
class Player;

//===========================================================
// ロックオン入力と対象選択を担当する
//===========================================================
class LockOnController : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	LockOnController()				= default;
	~LockOnController()	override	= default;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void PreUpdate()	override;

	//===========================================================
	// 公開関数群
	//===========================================================
	void SetCameraManager(const std::shared_ptr<CameraManager>& cameraManager)
	{
		m_wpCameraManager = cameraManager;
	}
	void SetPlayer(const std::shared_ptr<Player>& player)
	{
		m_wpPlayer = player;
	}

private:

	//===========================================================
	// 内部関数群
	//===========================================================
	// 画面内にいる最も近い対象を探す
	std::shared_ptr<ILockOnTarget> FindClosestTargetOnScreen() const;

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::weak_ptr<CameraManager>	m_wpCameraManager;
	std::weak_ptr<Player>			m_wpPlayer;

	//===========================================================
	// 状態値
	//===========================================================
	bool	m_wasLockOnKeyDown = false;
};