#pragma once

class CameraManager;
class ILockOnTarget;
class LockOnTargetManager;
class Player;

//===========================================================
// ロックオン入力・対象検索・対象切り替えを担当する
//===========================================================
class LockOnController : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	LockOnController() = default;
	~LockOnController()	override = default;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void PreUpdate() override;

	//===========================================================
	// 公開関数
	//===========================================================
	void SetCameraManager(const std::shared_ptr<CameraManager>& cameraManager)
	{
		m_wpCameraManager = cameraManager;
	}

	void SetLockOnTargetManager(
		const std::shared_ptr<LockOnTargetManager>& targetManager
	)
	{
		m_wpLockOnTargetManager = targetManager;
	}

	void SetPlayer(const std::shared_ptr<Player>& player)
	{
		m_wpPlayer = player;
	}

private:

	// 画面内候補と、その画面上の横位置をまとめる
	struct TargetCandidate
	{
		std::shared_ptr<ILockOnTarget>	target = nullptr;
		float							screenX = 0.0f;
	};

	//===========================================================
	// 内部関数
	//===========================================================
	// ロック可能かつ画面内にいる対象を左から右へ並べて返す
	std::vector<TargetCandidate> FindTargetsOnScreen() const;

	// 画面内からプレイヤーに最も近い対象を探す
	std::shared_ptr<ILockOnTarget> FindClosestTargetOnScreen() const;

	// 現在の対象から左右に隣接する対象を探す
	std::shared_ptr<ILockOnTarget> FindAdjacentTargetOnScreen(
		const std::shared_ptr<ILockOnTarget>& currentTarget,
		int direction
	) const;

	//===========================================================
	// 参照するオブジェクト
	//===========================================================
	std::weak_ptr<CameraManager>			m_wpCameraManager;
	std::weak_ptr<LockOnTargetManager>	m_wpLockOnTargetManager;
	std::weak_ptr<Player>					m_wpPlayer;

	//===========================================================
	// 入力状態
	//===========================================================
	bool m_wasLockOnKeyDown = false;
};