#pragma once

//===========================================================
// ステートマシンクラス
// 状態ごとの開始・更新・終了処理を登録し、現在状態の更新と遷移を管理する
// TStateId には enum class など、状態を識別する型を指定する
//===========================================================
template<class TStateId>
class StateMachine
{
public:

	//===========================================================
	// 状態処理の定義
	//===========================================================
	using EnterFunction		=	std::function<void()>;
	using UpdateFunction	=	std::function<void(float)>;
	using ExitFunction		=	std::function<void()>;

	// 1つの状態に紐づく開始・更新・終了処理
	struct StateFunctions
	{
		EnterFunction	enter;
		UpdateFunction	update;
		ExitFunction	exit;
	};

	//===========================================================
	// 公開関数
	//===========================================================
	
	// 状態を登録する
	void RegisterState(
		TStateId		stateID,
		EnterFunction	enter,
		UpdateFunction	update,
		ExitFunction	exit
	)
	{
		StateFunctions funcs{ enter, update, exit };
		m_states[stateID] = funcs;
	}

	// 状態を開始する
	void Start(TStateId initialState)
	{
		ChangeState(initialState);
	}

	// 状態を更新する
	void Update(float deltaTime)
	{
		
		auto stateIt = m_states.find(m_currentState);

		if (stateIt == m_states.end())	return;
		if (!stateIt->second.update)	return;

		stateIt->second.update(deltaTime);
	}

	// 状態を変更する
	void ChangeState(TStateId nextState)
	{
		auto nextStateIt = m_states.find(nextState);

		// 御堂録の状態には移動しない
		if (nextStateIt == m_states.end())	return;

		// 現在と同じ状態には入り直さない
		if (m_hasCurrentState && m_currentState == nextState)return;

		// 現在状態を終了する
		if (m_hasCurrentState)
		{
			auto currentStateIt = m_states.find(m_currentState);

			if (currentStateIt != m_states.end() &&
				currentStateIt->second.exit)
			{
				currentStateIt->second.exit();
			}
		}

		// 次の状態を現在状態として確定する
		m_currentState		= nextState;
		m_hasCurrentState	= true;

		// 次状態の開始処理を 1回呼ぶ
		if (nextStateIt->second.enter)
		{
			nextStateIt->second.enter();
		}
	}

	// 指定した状態が現在状態か調べる
	bool IsCurrentState(TStateId stateId) const
	{
		return m_hasCurrentState && m_currentState == stateId;
	}

private:

	//===========================================================
	// 状態値
	//===========================================================
	std::map<TStateId, StateFunctions>	m_states;
	TStateId							m_currentState{};
	bool								m_hasCurrentState = false;
};