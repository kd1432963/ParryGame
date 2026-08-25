#include "PlayerController.h"

#include "../Player/Player.h"
#include "../../GameObject/Camera/CameraBase.h"
#include "../../Combat/ILockOnTarget.h"
#include "../../Combat/LockOn/LockOnTargetManager.h"

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
	UpdateFacing();
	UpdateParry();
}

//===========================================================
// プレイヤー入力の有効・無効を切り替える
//===========================================================
void PlayerController::SetInputEnabled(bool isEnabled)
{
	m_isInputEnabled = isEnabled;

	if (!m_isInputEnabled) ClearInput();
}

//===========================================================
// 入力状態をクリアする関数
//===========================================================
void PlayerController::ClearInput()
{
	const auto player = m_wpPlayer.lock();

	if (!player) return;

	player->SetMoveDirection(Math::Vector3::Zero);
	player->SetFacingDirection(Math::Vector3::Zero);
	player->SetDashInput(false);
}

//===========================================================
// 移動入力の更新
//===========================================================
void PlayerController::UpdateMove()
{
	const auto	player			= m_wpPlayer.lock();
	const auto	camera			= m_wpCamera.lock();
	const auto	targetManager	= m_wpLockOnTargetManager.lock();

	if (!player || !camera) return;

	Math::Vector3	moveDirection		= Math::Vector3::Zero;
	Math::Vector3	moveForward			= Math::Vector3::Zero;
	Math::Vector3	moveRight			= Math::Vector3::Zero;
	bool			isLockOnMovement	= false;

	if (targetManager)
	{
		const auto target = targetManager->GetTarget();

		if (target)
		{
			// ロック中はプレイヤーから敵への方向を前方向にする
			moveForward = target->GetLockOnPosition() - player->GetPos();
			moveForward.y = 0.0f;

			if (moveForward.LengthSquared() > 0.0001f)
			{
				moveForward.Normalize();

				// 敵方向に対して直角となる右方向を作る
				moveRight =
				{
					moveForward.z,
					0.0f,
					-moveForward.x
				};

				isLockOnMovement = true;
			}
		}
	}

	// ロックしていない場合はカメラ基準で移動する
	if (!isLockOnMovement)
	{
		const Math::Matrix cameraMatrix = camera->GetMatrix();

		moveForward		= cameraMatrix.Backward();
		moveRight		= cameraMatrix.Right();

		moveForward.y	= 0.0f;
		moveRight.y		= 0.0f;

		moveForward.Normalize();
		moveRight.Normalize();
	}

	if (GetAsyncKeyState('W') & 0x8000) { moveDirection += moveForward; }
	if (GetAsyncKeyState('S') & 0x8000) { moveDirection -= moveForward; }
	if (GetAsyncKeyState('A') & 0x8000) { moveDirection -= moveRight; }
	if (GetAsyncKeyState('D') & 0x8000) { moveDirection += moveRight; }

	// 斜め移動だけ速くなることを防ぐ
	if (moveDirection.LengthSquared() > 1.0f)
	{
		moveDirection.Normalize();
	}

	const bool isDashPressed = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;

	player->SetMoveDirection(moveDirection);
	player->SetDashInput(isDashPressed);
}

//===========================================================
// ロックオン中の向きを更新する
//===========================================================
void PlayerController::UpdateFacing()
{
	const auto player = m_wpPlayer.lock();

	if (!player) return;

	const auto targetManager = m_wpLockOnTargetManager.lock();

	if (!targetManager)
	{
		player->SetFacingDirection(Math::Vector3::Zero);
		return;
	}

	const auto target = targetManager->GetTarget();

	if (!target)
	{
		player->SetFacingDirection(Math::Vector3::Zero);
		return;
	}

	Math::Vector3 facingDirection = target->GetLockOnPosition() - player->GetPos();

	facingDirection.y = 0.0f;

	if (facingDirection.LengthSquared() <= 0.0001f)
	{
		player->SetFacingDirection(Math::Vector3::Zero);
		return;
	}

	facingDirection.Normalize();
	player->SetFacingDirection(facingDirection);
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

	player->RequestParry();
}