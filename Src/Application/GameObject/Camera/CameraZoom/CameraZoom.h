#pragma once

//===========================================================
// 1回の画角変化に必要な調整値
//===========================================================
struct CameraZoomSettings
{
	float	targetFieldOfView	= 48.0f;
	float	zoomInDuration		= 0.05f;
	float	holdDuration		= 0.06f;
	float	zoomOutDuration		= 0.25f;
};

//===========================================================
// 通常画角から一時的にズームし、滑らかに元へ戻す
//===========================================================
class CameraZoom
{
public:

	//===========================================================
	// 公開関数群
	//===========================================================
	// 通常時に使用する画角を設定する
	void SetBaseFieldOfView(float fieldOfView);

	// 通常画角の目標値だけを変更し、現在値を滑らかに近づける
	void SetBaseFieldOfViewTarget(float fieldOfView);

	// 指定した画角へのズームを開始する
	void Start(const CameraZoomSettings& settings);

	// ヒットストップに影響されない時間で画角変化を進める
	void Update(float unscaledDeltaTime);

	float GetFieldOfView() const{ return m_currentFieldOfView; }

private:

	enum class Phase
	{
		None,
		ZoomIn,
		Hold,
		ZoomOut
	};

	//===========================================================
	// 内部関数群
	//===========================================================
	// 0～1の変化を、開始と終了が滑らかな値へ変換する
	float SmoothStep(float rate) const;

	//===========================================================
	// 状態値
	//===========================================================
	CameraZoomSettings	m_settings;
	Phase				m_phase					= Phase::None;
	float				m_baseFieldOfView		= 60.0f;
	float				m_startFieldOfView		= 60.0f;
	float				m_currentFieldOfView	= 60.0f;
	float				m_phaseElapsed			= 0.0f;
};
