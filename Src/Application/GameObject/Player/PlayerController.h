#pragma once

class Player;
class CameraBase;

//===========================================================
// クラスの役割
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
	void Update()	override;

	//===========================================================
	// 公開関数
	//===========================================================
	void SetPlayer(const std::shared_ptr<Player>& player)		{ m_wpPlayer = player; }
	void SetCamera(const std::shared_ptr<CameraBase>& camera)	{ m_wpCamera = camera; }

private:

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::weak_ptr<Player>		m_wpPlayer;
	std::weak_ptr<CameraBase>	m_wpCamera;
};