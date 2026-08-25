#include "CameraZoom.h"

namespace
{
	constexpr float	kMinimumFieldOfView			= 1.0f;
	constexpr float	kMaximumFieldOfView			= 179.0f;
	constexpr float	kMinimumDuration			= 0.0001f;
	constexpr float	kBaseFieldOfViewSharpness	= 8.0f;
}

//===========================================================
// 通常時に使用する画角を設定する
//===========================================================
void CameraZoom::SetBaseFieldOfView(float fieldOfView)
{
	m_baseFieldOfView = std::clamp(
		fieldOfView,
		kMinimumFieldOfView,
		kMaximumFieldOfView
	);

	if (m_phase != Phase::None) return;

	m_startFieldOfView		= m_baseFieldOfView;
	m_currentFieldOfView	= m_baseFieldOfView;
}

//===========================================================
// 通常時の画角の目標値を設定する
//===========================================================
void CameraZoom::SetBaseFieldOfViewTarget(float fieldOfView)
{
	m_baseFieldOfView = std::clamp(
		fieldOfView,
		kMinimumFieldOfView,
		kMaximumFieldOfView
	);
}

//===========================================================
// ズームを開始する
//===========================================================
void CameraZoom::Start(const CameraZoomSettings & settings)
{
	m_settings = settings;
	m_settings.targetFieldOfView = std::clamp(
		m_settings.targetFieldOfView,
		kMinimumFieldOfView,
		kMaximumFieldOfView
	);
	m_settings.zoomInDuration	= std::max(settings.zoomInDuration, kMinimumDuration);
	m_settings.holdDuration		= std::max(settings.holdDuration, 0.0f);
	m_settings.zoomOutDuration	= std::max(settings.zoomOutDuration, kMinimumDuration);

	// 連続して開始された場合も、現在の画角から自然につなげる
	m_startFieldOfView	= m_currentFieldOfView;
	m_phaseElapsed		= 0.0f;
	m_phase				= Phase::ZoomIn;
}

//===========================================================
// 更新関数
//===========================================================
void CameraZoom::Update(float unscaledDeltaTime)
{
	// 画角変化が発生していないときは、現在値を通常画角へ滑らかに近づける
	if (m_phase == Phase::None)
	{
		const float rate = 1.0f - expf(-kBaseFieldOfViewSharpness * std::max(unscaledDeltaTime, 0.0f));

		m_currentFieldOfView += (m_baseFieldOfView - m_currentFieldOfView) * rate;
		return;
	}

	m_phaseElapsed += std::max(unscaledDeltaTime, 0.0f);

	// ズームイン中は、開始画角から目標画角まで滑らかに変化させる
	if (m_phase == Phase::ZoomIn)
	{
		const float rate = SmoothStep(m_phaseElapsed / m_settings.zoomInDuration);

		m_currentFieldOfView = std::lerp(
			m_startFieldOfView,
			m_settings.targetFieldOfView,
			rate
		);

		if (m_phaseElapsed < m_settings.zoomInDuration) return;

		m_currentFieldOfView	= m_settings.targetFieldOfView;
		m_phaseElapsed			= 0.0f;
		m_phase					= Phase::Hold;
		return;
	}

	// ホールド中は、目標画角を維持する
	if (m_phase == Phase::Hold)
	{
		if (m_phaseElapsed < m_settings.holdDuration) return;

		m_phaseElapsed	= 0.0f;
		m_phase			= Phase::ZoomOut;
		return;
	}

	const float rate = SmoothStep(
		m_phaseElapsed / m_settings.zoomOutDuration
	);

	m_currentFieldOfView = std::lerp(
		m_settings.targetFieldOfView,
		m_baseFieldOfView,
		rate
	);

	if (m_phaseElapsed < m_settings.zoomOutDuration) return;

	m_currentFieldOfView	= m_baseFieldOfView;
	m_phaseElapsed			= 0.0f;
	m_phase					= Phase::None;
}

//===========================================================
// 0 ～ 1 の変化を、開始と終了が滑らかな値へ変換する
//===========================================================
float CameraZoom::SmoothStep(float rate) const
{
	rate = std::clamp(rate, 0.0f, 1.0f);

	return rate * rate * (3.0f - 2.0f * rate);
}
