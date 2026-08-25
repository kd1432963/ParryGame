#pragma once

class CameraBase;
class LockOnTargetManager;
class Player;

//===========================================================
// プレイヤーの入力を受け取り、プレイヤーとカメラに命令するクラス
//===========================================================
class PlayerController : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	PlayerController();
	~PlayerController() override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void PreUpdate()	override;

	//===========================================================
	// 公開関数
	//===========================================================
	void SetPlayer(const std::shared_ptr<Player>& player)		{ m_wpPlayer = player; }
	void SetCamera(const std::shared_ptr<CameraBase>& camera)	{ m_wpCamera = camera; }
	void SetInputEnabled(bool isEnabled);
	void SetLockOnTargetManager(
		const std::shared_ptr<LockOnTargetManager>& targetManager
	)
	{
		m_wpLockOnTargetManager = targetManager;
	}

private:

	//===========================================================
	// 内部関数
	//===========================================================
	void UpdateMove();
	void UpdateFacing();
	void UpdateParry();
	void ClearInput();

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::weak_ptr<Player>				m_wpPlayer;
	std::weak_ptr<CameraBase>			m_wpCamera;
	std::weak_ptr<LockOnTargetManager>	m_wpLockOnTargetManager;

	//===========================================================
	// 状態値
	//===========================================================
	bool	m_wasParryKeyDown	= false;
	bool	m_isInputEnabled	= true;
};