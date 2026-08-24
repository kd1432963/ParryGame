#pragma once

//===========================================================
// パリィの受付時間とリカバリー時間を管理する
//===========================================================
class ParrySystem
{
private:

	enum class ParryState
	{
		Ready,			// パリィ受付可能状態
		ParryAccept,	// パリィ受付中
		ParryRecovery,	// パリィ後のリカバリー中
	};

public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	ParrySystem();
	~ParrySystem();

	//===========================================================
	// 公開関数
	//===========================================================
	bool Start();
	void Update(float deltaTime);
	void Success();
	void Cancel();

	// パリィ受付中かを返す
	bool IsActive() const
	{
		return m_parryState == ParryState::ParryAccept;
	}

	// パリィ受付中か、リカバリー中かを返す
	bool IsBusy() const
	{
		return m_parryState != ParryState::Ready;
	}

	// パリィ入力後、攻撃を受け止められる秒数を返す
	static constexpr float GetActiveDuration()
	{
		return kActiveTime;
	}

private:

	//===========================================================
	// 状態値
	//===========================================================
	ParryState	m_parryState	= ParryState::Ready;
	float		m_timer			= 0.0f;

	//===========================================================
	// 固定値
	//===========================================================
	static constexpr float	kActiveTime		= 0.20f;
	static constexpr float	kRecoveryTime	= 0.35f;
};