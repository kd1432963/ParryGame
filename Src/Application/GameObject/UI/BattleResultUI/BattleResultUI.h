#pragma once

class BattleFlow;

//===========================================================
// BattleFlowの状態を参照し、戦闘終了時のUIを描画する
//===========================================================
class BattleResultUI : public KdGameObject
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	BattleResultUI()				= default;
	~BattleResultUI()	override	= default;

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	void Init()		override;
	void DrawSprite() override;

	//===========================================================
	// 公開関数群
	//===========================================================
	void SetBattleFlow(const std::shared_ptr<BattleFlow>& battleFlow)
	{
		m_wpBattleFlow = battleFlow;
	}

private:

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::weak_ptr<BattleFlow> m_wpBattleFlow;

	//===========================================================
	// 状態値
	//===========================================================
	Math::Vector2	m_clearTextSize		= Math::Vector2::Zero;
	Math::Vector2	m_gameOverTextSize	= Math::Vector2::Zero;
	Math::Vector2	m_retryTextSize		= Math::Vector2::Zero;
};
