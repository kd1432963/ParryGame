#pragma once

//============================================================
// パリィ成功時の Effekseer エフェクト再生をまとめる
//============================================================
class ParryEffect
{
public:

	// 攻撃と被弾判定が接触した座標にパリィエフェクトを再生する
	static void Play(const Math::Vector3& hitPosition);
};
