#include "CameraBase.h"

#include "CameraShake/CameraShake.h"
#include "CameraZoom/CameraZoom.h"
#include "../../main.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
CameraBase::CameraBase()
	:	m_upCameraShake(std::make_unique<CameraShake>()),
		m_upCameraZoom(std::make_unique<CameraZoom>())
{}
CameraBase::~CameraBase() = default;

//===========================================================
// 初期化関数
//===========================================================
void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	KdEffekseerManager::GetInstance().SetCamera(
		this->GetCamera()
	);
}

//===========================================================
// 更新関数
//===========================================================
void CameraBase::Update()
{
	const float deltaTime = Application::Instance().GetUnscaledDeltaTime();

#ifdef _DEBUG
	// F6を押した瞬間にテストシェイク
	if (GetAsyncKeyState(VK_F6) & 0x0001)
	{
		CameraShakeSettings settings{};

		settings.durationSeconds	= 0.50f;
		settings.samplesPerSecond	= 20.0f;

		// 確認しやすいように強め
		settings.maxPositionOffset	= Math::Vector3(0.25f, 0.15f, 0.0f);

		settings.maxRotationDegrees = Math::Vector3(1.0f, 1.0f, 1.5f);

		StartShake(settings);
	}
#endif

	m_upCameraShake->Update(deltaTime);
	m_upCameraZoom->Update(deltaTime);
}

//===========================================================
// 描画前処理
//===========================================================
void CameraBase::PreDraw()
{
	if (!m_spCamera) return;

	// ズームの設定
	m_spCamera->SetProjectionMatrix(m_upCameraZoom->GetFieldOfView());

	Math::Matrix cameraMat = m_mWorld;

	if (m_upCameraShake)
	{
		const CameraShakeOffset& offset = m_upCameraShake->GetOffset();

		// カメラローカル空間で回転シェイクを作る
		const Math::Matrix shakeRotation =
			Math::Matrix::CreateFromYawPitchRoll(
				DirectX::XMConvertToRadians(
					offset.rotationOffsetDegrees.y),
				DirectX::XMConvertToRadians(
					offset.rotationOffsetDegrees.x),
				DirectX::XMConvertToRadians(
					offset.rotationOffsetDegrees.z)
			);

		// カメラローカル空間で位置シェイクを作る
		const Math::Matrix shakeTranslation = Math::Matrix::CreateTranslation(offset.positionOffset);

		// 通常のカメラ行列へシェイクを追加
		cameraMat = shakeRotation * shakeTranslation * cameraMat;
	}

	m_spCamera->SetCameraMatrix(cameraMat);
	m_spCamera->SetToShader();
}

//===========================================================
// ターゲットの設定
//===========================================================
void CameraBase::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	if (!target) { return; }

	m_wpTarget = target;
}

//============================================================
// カメラシェイクを開始する
//============================================================
void CameraBase::StartShake(const CameraShakeSettings& settings)
{
	m_upCameraShake->Start(settings);
}

//============================================================
// カメラシェイクを開始する(簡易版)
//============================================================
void CameraBase::StartShake(float power, float time)
{
	CameraShakeSettings settings{};

	settings.durationSeconds	= time;
	settings.samplesPerSecond	= 30.0f;
	settings.maxPositionOffset	= Math::Vector3(power, power, 0.0f);

	StartShake(settings);
}

//============================================================
// カメラズームを開始する
//============================================================
void CameraBase::StartZoom(const CameraZoomSettings& settings)
{
	m_upCameraZoom->Start(settings);
}

//============================================================
// カメラ切り替え直後の準備
//============================================================
void CameraBase::OnActivated()
{
	// 切り替え中に動いたマウスを、次の回転入力として扱わない
	ResetMouseInput();
}

//===========================================================
// マウスでカメラを回転させる
//===========================================================
void CameraBase::UpdateRotateByMouse()
{
	const HWND windowHandle = Application::Instance().GetWindowHandle();

	if (GetForegroundWindow() != windowHandle)
	{
		ResetMouseInput();
		return;
	}

	// クライアント領域の中央をスクリーン座標へ変換する
	RECT clientRect{};
	GetClientRect(windowHandle, &clientRect);

	POINT centerPos
	{
		(clientRect.right - clientRect.left) / 2,
		(clientRect.bottom - clientRect.top) / 2
	};

	ClientToScreen(windowHandle, &centerPos);

	// 中央からどれだけマウスが移動したか求める
	POINT currentPos{};
	GetCursorPos(&currentPos);

	// 初回とロックオン解除直後は中央へ戻すだけにする
	if (!m_isMouseInputReady)
	{
		SetCursorPos(centerPos.x, centerPos.y);
		m_isMouseInputReady = true;
		return;
	}

	const POINT mouseMove
	{
		currentPos.x - centerPos.x,
		currentPos.y - centerPos.y
	};

	SetCursorPos(centerPos.x, centerPos.y);

	// マウス移動量を角度へ変換する
	constexpr float kMouseSensitivity = 0.15f;

	m_DegAng.x += mouseMove.y * kMouseSensitivity;
	m_DegAng.y += mouseMove.x * kMouseSensitivity;

	// 真上・真下まで回り込まないように制限する
	m_DegAng.x = std::clamp(m_DegAng.x, -45.0f, 45.0f);
}