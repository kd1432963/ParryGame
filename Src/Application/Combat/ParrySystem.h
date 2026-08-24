#pragma once

//===========================================================
// パリィの受付時間とリカバリー時間を管理する
//===========================================================
class ParrySystem
{
private:

	enum class ParryState
	{
		None,			// パリィ受付中ではない
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
	// パリィを開始できた場合だけtrueを返す
	bool Start();

	// 受付時間とリカバリー時間を進める
	void Update(float deltaTime);

	// パリィ成功時に受付を終了する
	void Success();

	// 別の状態へ遷移したとき、パリィを即座に終了する
	void Cancel();

	bool IsActive() const
	{
		return m_parryState == ParryState::ParryAccept;
	}

	bool IsBusy() const
	{
		return m_parryState != ParryState::None;
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
	ParryState	m_parryState	= ParryState::None;
	float		m_timer			= 0.0f;

	//===========================================================
	// 固定値
	//===========================================================
	static constexpr float	kActiveTime		= 0.20f;
	static constexpr float	kRecoveryTime	= 0.35f;
};