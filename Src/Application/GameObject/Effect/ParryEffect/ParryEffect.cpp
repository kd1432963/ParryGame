#include "ParryEffect.h"

//===========================================================
// パリィ成功時のエフェクト再生
//===========================================================
void ParryEffect::Play(const Math::Vector3& hitPosition)
{
	// この再生処理だけで使うため、実装内に閉じ込める
	static const std::string	kEffectFileName		= "Parry/Parry.efkefc";
	static const std::string	kSoundFileName		= "Asset/Sounds/SE/Parry.wav";
	constexpr float				kEffectScale		= 1.0f;
	constexpr float				kSoundVolume		= 0.05f;
	constexpr float				kSoundPitch			= 0.05f;

	KdEffekseerManager::GetInstance().Play(
		kEffectFileName,
		hitPosition,
		kEffectScale
	);

	// 成功の手応えを優先し、距離減衰しない 2D 効果音として再生する
	const auto sound = KdAudioManager::Instance().Play(kSoundFileName);

	if (!sound) return;

	sound->SetVolume(kSoundVolume);
	sound->SetPitch(kSoundPitch);
}
