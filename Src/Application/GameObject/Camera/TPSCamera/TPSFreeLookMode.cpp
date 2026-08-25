#include "TPSFreeLookMode.h"

#include "../../../main.h"

namespace
{
	// マウス移動量に掛ける回転感度
	constexpr float	kMouseSensitivity = 0.15f;

	// カメラが真上・真下まで回り込まないための角度制限
	constexpr float	kMinPitch = -45.0f;
	constexpr float	kMaxPitch = 45.0f;
}

void TPSFreeLookMode::Update(Math::Vector3& cameraAngle)
{
	const HWND windowHandle = Application::Instance().GetWindowHandle();

	// ゲーム画面が選択されていない間は入力を受け取らない
	if (GetForegroundWindow() != windowHandle)
	{
		ResetInput();
		return;
	}

	// ウィンドウ中央のスクリーン座標を求める
	RECT clientRect{};
	GetClientRect(windowHandle, &clientRect);

	POINT centerPosition
	{
		(clientRect.right - clientRect.left) / 2,
		(clientRect.bottom - clientRect.top) / 2
	};

	ClientToScreen(windowHandle, &centerPosition);

	POINT currentPosition{};
	GetCursorPos(&currentPosition);

	// 初回は中央へ戻すだけにして、大きな回転を防ぐ
	if (!m_isMouseInputReady)
	{
		SetCursorPos(centerPosition.x, centerPosition.y);
		m_isMouseInputReady = true;
		return;
	}

	const POINT mouseMovement
	{
		currentPosition.x - centerPosition.x,
		currentPosition.y - centerPosition.y
	};

	SetCursorPos(centerPosition.x, centerPosition.y);

	// マウス移動量をカメラ角度へ変換する
	cameraAngle.x += mouseMovement.y * kMouseSensitivity;
	cameraAngle.y += mouseMovement.x * kMouseSensitivity;

	cameraAngle.x = std::clamp(
		cameraAngle.x,
		kMinPitch,
		kMaxPitch
	);
}

void TPSFreeLookMode::ResetInput()
{
	m_isMouseInputReady = false;
}