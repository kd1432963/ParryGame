#include "CameraShake.h"

namespace
{
	// 処理落ちやブレークポイント復帰後に大量更新されることを防ぐ
	constexpr float kMaxDeltaTime = 0.1f;

	bool HasStrength(const Math::Vector3& strength)
	{
		return strength.LengthSquared() > 0.0f;
	}

	float SmoothStep(float value)
	{
		value = std::clamp(value, 0.0f, 1.0f);
		return value * value * (3.0f - 2.0f * value);
	}

	Math::Vector3 CreateRandomVector(const Math::Vector3& strength)
	{
		const float	strengthX = std::abs(strength.x);
		const float	strengthY = std::abs(strength.y);
		const float	strengthZ = std::abs(strength.z);

		return Math::Vector3(
			KdRandom::GetFloat(-strengthX, strengthX),
			KdRandom::GetFloat(-strengthY, strengthY),
			KdRandom::GetFloat(-strengthZ, strengthZ)
		);
	}
}

//===========================================================
// カメラシェイクの開始
//===========================================================
void CameraShake::Start(const CameraShakeSettings& settings)
{
	if(	settings.durationSeconds <= 0.0f ||
		settings.samplesPerSecond <= 0.0f)
	{
		return;
	}
	if(	!HasStrength(settings.maxPositionOffset) &&
		!HasStrength(settings.maxRotationDegrees))
	{
		return;
	}
	
	// 再生中に新しい揺れが来ても現在位置から滑らかにつなぐ
	m_sampleFromPosition = m_offset.positionOffset;
	m_sampleFromRotation = m_offset.rotationOffsetDegrees;

	m_settings					= settings;
	m_settings.samplesPerSecond = std::clamp(settings.samplesPerSecond, 1.0f, 120.0f);
	m_remainingTime				= settings.durationSeconds;
	m_sampleElapsed				= 0.0f;

	CreateNextSample();
}

//===========================================================
// カメラシェイクの更新
//===========================================================
void CameraShake::Update(float deltaTime)
{
	// シェイク停止中、または経過時間が無効なら更新しない
	if (!IsActive() || deltaTime <= 0.0f) return;

	// 大きなフレーム遅延による急激な変化を防ぐ
	deltaTime = std::min(deltaTime, kMaxDeltaTime);

	// シェイクの残り時間を更新する
	m_remainingTime = std::max(0.0f, m_remainingTime - deltaTime);

	// 再生時間が終了したら、各種状態を初期化する
	if (!IsActive())
	{
		Stop();
		return;
	}

	// ランダムな揺れの目標値を更新する間隔
	// 前提として samplesPerSecond は 0 より大きい必要がある
	const float sampleDuration = 1.0f / m_settings.samplesPerSecond;

	m_sampleElapsed += deltaTime;

	// 更新間隔を超えるたびに次のランダム値を生成する
	// while にすることで、低フレームレート時の更新漏れを防ぐ
	while (m_sampleElapsed >= sampleDuration)
	{
		m_sampleElapsed -= sampleDuration;

		// 現在の目標値を、次の補間区間の開始値として引き継ぐ
		m_sampleFromPosition = m_sampleToPosition;
		m_sampleFromRotation = m_sampleToRotation;

		// 次に向かうランダムな目標値を生成する
		CreateNextSample();
	}

	// 現在のサンプル区間内での進行率を求める
	// SmoothStepを通すことで、補間の始点と終点を滑らかにする
	const float sampleRate =
		SmoothStep(m_sampleElapsed / sampleDuration);

	// 残り時間に応じた減衰率
	// 終了時間に近づくほど、揺れの強さが0へ近づく
	const float decayRate =
		SmoothStep(m_remainingTime / m_settings.durationSeconds);

	// 2つのランダム値を補間し、フレームごとのガタつきを防ぐ
	m_offset.positionOffset = DirectX::XMVectorLerp(
		m_sampleFromPosition,
		m_sampleToPosition,
		sampleRate
	);

	m_offset.rotationOffsetDegrees = DirectX::XMVectorLerp(
		m_sampleFromRotation,
		m_sampleToRotation,
		sampleRate
	);

	// シェイク終了に向けて位置と回転の揺れを徐々に弱める
	m_offset.positionOffset			*= decayRate;
	m_offset.rotationOffsetDegrees	*= decayRate;
}

//===========================================================
// カメラシェイクの停止・状態初期化
//===========================================================
void CameraShake::Stop()
{
	// 前回のシェイク状態が残らないよう、すべて初期値へ戻す
	m_settings				= CameraShakeSettings{};
	m_offset				= CameraShakeOffset{};
	m_sampleFromPosition	= Math::Vector3::Zero;
	m_sampleToPosition		= Math::Vector3::Zero;
	m_sampleFromRotation	= Math::Vector3::Zero;
	m_sampleToRotation		= Math::Vector3::Zero;
	m_remainingTime			= 0.0f;
	m_sampleElapsed			= 0.0f;
}

//===========================================================
// 次の揺れの目標値を生成
//===========================================================
void CameraShake::CreateNextSample()
{
	// 設定された最大振幅の範囲内で、位置と回転の目標値を生成する
	m_sampleToPosition = CreateRandomVector(m_settings.maxPositionOffset);
	m_sampleToRotation = CreateRandomVector(m_settings.maxRotationDegrees);
}