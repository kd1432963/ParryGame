#include "CameraManager.h"

#include "CameraBase.h"
#include "CameraShake/CameraShake.h"
#include "CameraZoom/CameraZoom.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
CameraManager::CameraManager()
{}
CameraManager::~CameraManager() = default;

//===========================================================
// 更新関数
//===========================================================
void CameraManager::Update()
{
	if (!m_spActiveCamera) return;

	m_spActiveCamera->Update();
}

//===========================================================
// 更新後処理関数
//===========================================================
void CameraManager::PostUpdate()
{
	if (!m_spActiveCamera) return;

	m_spActiveCamera->PostUpdate();
}

//===========================================================
// 描画前処理関数
//===========================================================
void CameraManager::PreDraw()
{
	if (!m_spActiveCamera) return;

	m_spActiveCamera->PreDraw();
}

//===========================================================
// 種類と実体を登録し、最初の 1 台は自動的に使用中にする
//===========================================================
void CameraManager::RegisterCamera(
	CameraType							type, 
	const std::shared_ptr<CameraBase>&	camera)
{
	if (!camera) return;

	m_cameras[type] = camera;

	// 最初の 1 台は自動的に使用中にする
	if (!m_spActiveCamera)
	{
		ChangeCamera(type);
	}
}

//===========================================================
// 登録済みのカメラへ切り替え、成功した場合だけtrueを返す
//===========================================================
bool CameraManager::ChangeCamera(CameraType type)
{
	const auto cameraIt = m_cameras.find(type);

	if (cameraIt == m_cameras.end() || !cameraIt->second) return false;

	m_activeType		= type;
	m_spActiveCamera	= cameraIt->second;
	m_spActiveCamera->OnActivated();

	// 3D エフェクトも切り替え後のカメラから見た向きで描画する
	KdEffekseerManager::GetInstance().SetCamera(
		m_spActiveCamera->GetCamera()
	);

	return true;
}

//===========================================================
// プレイヤー移動などで使う、現在のカメラ行列を返す
//===========================================================
Math::Matrix CameraManager::GetCameraMatrix() const
{
	if (!m_spActiveCamera) return Math::Matrix::Identity;

	return m_spActiveCamera->GetMatrix();
}

//===========================================================
// 演出は呼び出した時点で使用中のカメラへ渡す
//===========================================================
void CameraManager::StartShake(const CameraShakeSettings& settings)
{
	if (!m_spActiveCamera) return;

	m_spActiveCamera->StartShake(settings);
}

//===========================================================
// 演出は呼び出した時点で使用中のカメラへ渡す
//===========================================================
void CameraManager::StartZoom(const CameraZoomSettings & settings)
{
	if (!m_spActiveCamera) return;

	m_spActiveCamera->StartZoom(settings);
}
