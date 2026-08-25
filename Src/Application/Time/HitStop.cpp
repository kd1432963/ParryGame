#include "HitStop.h"

//===========================================================
// ヒットストップを開始する
//===========================================================
void HitStop::Start(float durationSeconds, float timeScale)
{
	if (durationSeconds <= 0.0f) return;

	const float clampedTimeScale = std::clamp(timeScale, 0.0f, 1.0f);

	// 連続して要求された場合は、長くて強いほうを残す
	m_remainingSeconds	= std::max(m_remainingSeconds, durationSeconds);
	m_timeScale			= std::min(m_timeScale, clampedTimeScale);
}

//===========================================================
// ヒットストップを更新する
//===========================================================
void HitStop::Update(float unscaledDeltaTime)
{
	if (!IsActive() || unscaledDeltaTime <= 0.0f) return;

	m_remainingSeconds = std::max(0.0f,m_remainingSeconds - unscaledDeltaTime);

	if (m_remainingSeconds <= 0.0f)
	{
		Stop();
	}
}

//===========================================================
// ヒットストップを即座に終了する
//===========================================================
void HitStop::Stop()
{
	m_remainingSeconds	= 0.0f;
	m_timeScale			= 1.0f;
}