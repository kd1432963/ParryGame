#pragma once

enum class ParryResult
{
	ContinueCombo,	// 連撃継続
	Stagger,		// 連撃終了・怯み
	Defeated		// 撃破
};

//===========================================================
// パリィされたときの処理を持つオブジェクト用インターフェース
//===========================================================
class IParryable
{
public:

	IParryable()			= default;
	virtual ~IParryable()	= default;

	virtual ParryResult OnParried()						= 0;
	virtual int GetCurrentParryDurability()		const	= 0;
	virtual int GetMaxParryDurability()			const	= 0;
};