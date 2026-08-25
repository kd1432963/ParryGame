#pragma once

//===========================================================
// 1回の画面フラッシュに必要な調整値
//===========================================================
struct ScreenFlashSettings
{
	Math::Vector3	color			= { 1.0f, 0.85f, 0.40f };	// RGB値
	float			peakOpacity		= 0.22f; // 最大不透明度
	float			fadeDuration	= 0.14f; // フラッシュが消えるまでの時間
};

//===========================================================
// 画面全体へ一瞬だけ色を重ねて衝撃を強調する
//===========================================================
class ScreenFlash : public KdGameObject
{
public:

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Update()		override;
	void DrawSprite()	override;

	//===========================================================
	// 公開関数群
	//===========================================================
	// 指定した色と時間でフラッシュを開始する
	void Start(const ScreenFlashSettings& settings);

private:

	ScreenFlashSettings	m_settings;
	float				m_remainingTime		= 0.0f;
	float				m_currentOpacity	= 0.0f;
};
