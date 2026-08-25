#include "BattleResultUI.h"

#include "../../../GameFlow/BattleFlow.h"

namespace
{
	constexpr char kClearText[]		= "BATTLE CLEAR";
	constexpr char kGameOverText[]	= "GAME OVER";
	constexpr char kRetryText[]		= "PRESS ENTER";

	// 画面中央に表示するリザルトパネルの大きさ
	constexpr int kPanelHalfWidth	= 320;
	constexpr int kPanelHalfHeight	= 112;

	// 左下基準の文字列を配置するために、実際の描画サイズを取得する
	Math::Vector2 MeasureText(const char* text)
	{
		const auto fontSprite = KdFontManager::Instance().CreateFontTexture(0, text, false);

		if (!fontSprite || fontSprite->GetTexList().empty()) return Math::Vector2::Zero;

		float width = 0.0f;

		// DrawFontと同じく、描画する全グリフのテクスチャ幅を合計する
		for (const auto& fontData : fontSprite->GetTexList())
		{
			if (!fontData || !fontData->FontTex) continue;

			width += static_cast<float>(fontData->FontTex->GetInfo().Width);
		}

		const float height = static_cast<float>(
			fontSprite->GetTexList().front()->FontTex->GetInfo().Height
		);

		return { width, height };
	}

	// DrawFontは左下基準なので、文字列の中心が指定位置へ来る座標を作る
	Math::Vector2 GetCenteredTextPosition(
		const Math::Vector2& textSize,
		float centerY
	)
	{
		return
		{
			-textSize.x * 0.5f,
			centerY - textSize.y * 0.5f
		};
	}
}

//===========================================================
// 初期化関数
//===========================================================
void BattleResultUI::Init()
{
	// フォントごとの実幅を保存し、固定座標による中央ずれを防ぐ
	m_clearTextSize		= MeasureText(kClearText);
	m_gameOverTextSize	= MeasureText(kGameOverText);
	m_retryTextSize		= MeasureText(kRetryText);
}

//===========================================================
// 戦闘結果を描画する
//===========================================================
void BattleResultUI::DrawSprite()
{
	const auto battleFlow = m_wpBattleFlow.lock();

	if (!battleFlow) return;

	const BattleState state = battleFlow->GetState();

	// 戦闘中はリザルトUIを表示しない
	if (state == BattleState::Playing) return;

	const bool			isCleared		= state == BattleState::Cleared;
	const char*			resultText		= isCleared ? kClearText : kGameOverText;
	const Math::Vector2& resultTextSize	= isCleared ? m_clearTextSize : m_gameOverTextSize;
	const Math::Vector2	resultTextPos	= GetCenteredTextPosition(resultTextSize, 25.0f);
	const Math::Vector2	retryTextPos	= GetCenteredTextPosition(m_retryTextSize, -42.0f);
	const Math::Color	shadowColor		(0.00f, 0.00f, 0.00f, 0.65f);
	const Math::Color	panelColor		(0.01f, 0.025f, 0.055f, 0.92f);
	const Math::Color	panelEdgeColor	(0.08f, 0.13f, 0.19f, 0.95f);
	const Math::Color	accentColor		= isCleared
		? Math::Color(0.20f, 0.90f, 1.00f, 1.00f)
		: Math::Color(1.00f, 0.24f, 0.12f, 1.00f);
	const Math::Color	textColor		(0.78f, 0.84f, 0.90f, 1.00f);

	auto& spriteShader = KdShaderManager::Instance().m_spriteShader;

	// 背景・外枠・状態色のラインを重ねてパネルを構成する
	spriteShader.DrawBox(4, -4, kPanelHalfWidth, kPanelHalfHeight, &shadowColor);
	spriteShader.DrawBox(0, 0, kPanelHalfWidth, kPanelHalfHeight, &panelColor);
	spriteShader.DrawBox(0, 0, kPanelHalfWidth, kPanelHalfHeight, &panelEdgeColor, false);
	spriteShader.DrawBox(0, 72, kPanelHalfWidth - 18, 2, &accentColor);
	spriteShader.DrawBox(-kPanelHalfWidth + 14, 72, 8, 8, &accentColor);
	spriteShader.DrawBox(kPanelHalfWidth - 14, 72, 8, 8, &accentColor);

	// 影を先に描いて文字を背景から浮かせる
	spriteShader.DrawFont(
		resultTextPos + Math::Vector2(3.0f, -3.0f),
		&shadowColor,
		"%s",
		resultText
	);
	spriteShader.DrawFont(
		resultTextPos,
		&accentColor,
		"%s",
		resultText
	);
	spriteShader.DrawFont(
		retryTextPos,
		&textColor,
		kRetryText
	);
}
