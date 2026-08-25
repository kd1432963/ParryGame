#pragma once

class CameraBase;
struct CameraShakeSettings;
struct CameraZoomSettings;

//===========================================================
// シーンで使用するカメラの所有と切り替えを管理する
//===========================================================
class CameraManager : public KdGameObject
{
public:

	enum class CameraType
	{
		ThirdPerson,
		FirstPerson,
		Event
	};

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	CameraManager();
	~CameraManager()	override;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Update()		override;
	void PostUpdate()	override;
	void PreDraw()		override;

	//===========================================================
	// 公開関数群
	//===========================================================
	// 種類と実体を登録し、最初の1台は自動的に使用中にする
	void RegisterCamera(
		CameraType type,
		const std::shared_ptr<CameraBase>& camera
	);

	// 登録済みのカメラへ切り替え、成功した場合だけtrueを返す
	bool ChangeCamera(CameraType type);

	const std::shared_ptr<CameraBase>& GetActiveCamera() const{ return m_spActiveCamera; }

	// プレイヤー移動などで使う、現在のカメラ行列を返す
	Math::Matrix GetCameraMatrix() const;

	// 演出は呼び出した時点で使用中のカメラへ渡す
	void StartShake(const CameraShakeSettings& settings);
	void StartZoom(const CameraZoomSettings& settings);

	CameraType GetActiveCameraType() const{ return m_activeType; }

private:

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::map<CameraType, std::shared_ptr<CameraBase>>	m_cameras;
	std::shared_ptr<CameraBase>							m_spActiveCamera = nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	CameraType m_activeType = CameraType::ThirdPerson;
};
