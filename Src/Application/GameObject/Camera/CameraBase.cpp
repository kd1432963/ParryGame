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
}

//===========================================================
// 更新関数
//===========================================================
void CameraBase::Update()
{
	const float deltaTime = Application::Instance().GetDeltaTime();

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

//===========================================================
// マウスでカメラを回転させる
//===========================================================
void CameraBase::UpdateRotateByMouse()
{
	// マウスでカメラを回転させる処理
	POINT _nowPos;
	GetCursorPos(&_nowPos);

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 実際にカメラを回転させる処理(0.15はただの補正値)
	m_DegAng.x += _mouseMove.y * 0.15f;
	m_DegAng.y += _mouseMove.x * 0.15f;

	// 回転制御
	m_DegAng.x = std::clamp(m_DegAng.x, -45.f, 45.f);
}