#include "PlayerController.h"

#include "../Player/Player.h"
#include "../../GameObject/Camera/CameraBase.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
PlayerController::PlayerController()	= default;
PlayerController::~PlayerController()	= default;

//===========================================================
// 更新関数
//===========================================================
void PlayerController::PreUpdate()
{
	if (!m_isInputEnabled)
	{
		ClearInput();
		return;
	}

	UpdateMove();
	UpdateParry();
}

//===========================================================
// 入力状態をクリアする関数
//===========================================================
void PlayerController::ClearInput()
{
	const auto player = m_wpPlayer.lock();

	if (!player) return;

	player->SetMoveDirection(Math::Vector3::Zero);
	player->SetDashInput(false);
}

//===========================================================
// 移動入力の更新
//===========================================================
void PlayerController::UpdateMove()
{
	auto player = m_wpPlayer.lock();
	auto camera = m_wpCamera.lock();

	if (!player || !camera) return;

	Math::Vector3	moveDir = Math::Vector3::Zero;

	// カメラの向きから前方向と右方向を取得
	Math::Matrix	cameraMat		= camera->GetMatrix();
	Math::Vector3	cameraForward	= cameraMat.Backward();
	Math::Vector3	cameraRight		= cameraMat.Right();

	// 地面移動なのでYは不要
	cameraForward.y = 0.0f;
	cameraRight.y	= 0.0f;

	// 正規化
	cameraForward.Normalize();
	cameraRight.Normalize();

	// 入力
	if (GetAsyncKeyState('W') & 0x8000) { moveDir += cameraForward; }
	if (GetAsyncKeyState('S') & 0x8000) { moveDir -= cameraForward; }
	if (GetAsyncKeyState('A') & 0x8000) { moveDir -= cameraRight; }
	if (GetAsyncKeyState('D') & 0x8000) { moveDir += cameraRight; }

	// 斜め移動対策
	if (moveDir.LengthSquared() > 1.0f) { moveDir.Normalize(); }

	const bool isDashPressed = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;

	// Player へ命令
	player->SetMoveDirection(moveDir);
	player->SetDashInput(isDashPressed);
}

//===========================================================
// 移動入力の更新
//===========================================================
void PlayerController::UpdateParry()
{
	const bool	isParryKeyDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	const bool	isParryPressed = isParryKeyDown && !m_wasParryKeyDown;

	m_wasParryKeyDown = isParryKeyDown;

	if (!isParryPressed) return;

	const auto player = m_wpPlayer.lock();

	if (!player) return;

	//player->RequestParry();
}