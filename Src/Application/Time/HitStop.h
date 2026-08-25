#pragma once

//============================================================
// 攻撃命中時の一時停止時間とゲーム速度を管理する
//============================================================
class HitStop
{
public:

	//===========================================================
	// 公開関数群
	//===========================================================
	// 指定時間だけゲーム速度を変更する
	void Start(float durationSeconds, float timeScale);

	// 通常時間を使って残り時間を進める
	void Update(float unscaledDeltaTime);

	// ヒットストップを即座に終了する
	void Stop();

	bool IsActive()			const{ return m_remainingSeconds > 0.0f;}
	float GetTimeScale()	const{ return IsActive() ? m_timeScale : 1.0f;}

private:

	//===========================================================
	// 状態値
	//===========================================================
	// 停止が解除されるまでの残り秒数
	float	m_remainingSeconds = 0.0f;

	// 0.0 なら完全停止、0.5 なら半分の速度
	float	m_timeScale = 1.0f;
};