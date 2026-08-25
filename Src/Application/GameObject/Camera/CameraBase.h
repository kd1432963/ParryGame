#pragma once

class CameraShake;
class CameraZoom;
struct CameraShakeSettings;
struct CameraZoomSettings;

//============================================================
// すべての 3D カメラに共通する機能
//============================================================
class CameraBase : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	CameraBase();
	virtual ~CameraBase()	override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Init()				override;
	void Update()			override;
	void PreDraw()			override;

	//===========================================================
	// 公開関数群
	//===========================================================
	void SetTarget(const std::shared_ptr<KdGameObject>& target);
	void StartShake(const CameraShakeSettings& settings);
	void StartShake(float power, float time);
	void StartZoom(const CameraZoomSettings& settings);

	// 「絶対変更しません！見るだけ！」な書き方
	const std::shared_ptr<KdCamera>& GetCamera() const
	{
		return m_spCamera;
	}

	// 「中身弄るかもね」な書き方
	std::shared_ptr<KdCamera> WorkCamera() const
	{
		return m_spCamera;
	}

	const Math::Matrix GetRotationMatrix()const
	{
		return	Math::Matrix::CreateFromYawPitchRoll(
				DirectX::XMConvertToRadians(m_DegAng.y),
				DirectX::XMConvertToRadians(m_DegAng.x),
				DirectX::XMConvertToRadians(m_DegAng.z));
	}

	const Math::Matrix GetRotationYMatrix() const
	{
		return Math::Matrix::CreateRotationY(
			   DirectX::XMConvertToRadians(m_DegAng.y));
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

protected:

	//===========================================================
	// 内部関数群
	//===========================================================
	void UpdateRotateByMouse();

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::shared_ptr<KdCamera>					m_spCamera		= nullptr;
	std::weak_ptr<KdGameObject>					m_wpTarget;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};
	std::unique_ptr<CameraShake>				m_upCameraShake = nullptr;
	std::unique_ptr<CameraZoom>					m_upCameraZoom	= nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector3	m_DegAng	= Math::Vector3::Zero;
	Math::Matrix	m_mLocalPos = Math::Matrix::Identity;
	Math::Matrix	m_mRotation = Math::Matrix::Identity;

	// カメラ回転用マウス座標の差分
	POINT			m_FixMousePos{};
};