#include "ScreenFlash.h"

#include "../../../main.h"

//===========================================================
// 更新関数
//===========================================================
void ScreenFlash::Update()
{
	if (m_remainingTime <= 0.0f) return;

	m_remainingTime = std::max(0.0f,m_remainingTime - Application::Instance().GetUnscaledDeltaTime());

	const float remainRate = m_remainingTime / m_settings.fadeDuration;

	// 前半は明るさを保ち、後半へ向かって素早く透明にする
	m_currentOpacity = m_settings.peakOpacity * remainRate * remainRate;
}

//===========================================================
// 描画関数
//===========================================================
void ScreenFlash::DrawSprite()
{
	if (m_currentOpacity <= 0.0f) return;

	// クライアント領域のサイズを取得
	RECT clientRect{};
	GetClientRect(Application::Instance().GetWindowHandle(),&clientRect);

	const int	halfWidth	= (clientRect.right - clientRect.left) / 2;
	const int	halfHeight	= (clientRect.bottom - clientRect.top) / 2;
	const Math::Color flashColor(
		m_settings.color.x,
		m_settings.color.y,
		m_settings.color.z,
		m_currentOpacity
	);

	// 画面全体にフラッシュを描画
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0,
		0,
		halfWidth,
		halfHeight,
		&flashColor,
		true
	);
}

//===========================================================
// 指定した色と時間でフラッシュを開始する
//===========================================================
void ScreenFlash::Start(const ScreenFlashSettings & settings)
{
	// フェード時間が極端に短いと、フレームレートの影響で不自然な見え方になるため、最小値を設定する
	constexpr float kMinimumDuration = 0.0001f;

	// 設定値を保存する
	m_settings				= settings;
	m_settings.peakOpacity	= std::clamp(settings.peakOpacity, 0.0f, 1.0f);
	m_settings.fadeDuration = std::max(settings.fadeDuration, kMinimumDuration);
	m_remainingTime			= m_settings.fadeDuration;
	m_currentOpacity		= m_settings.peakOpacity;
}
