#include "ParrySystem.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
ParrySystem::ParrySystem()	= default;
ParrySystem::~ParrySystem() = default;

//===========================================================
// 移動入力の更新
//===========================================================
bool ParrySystem::Start()
{
	if (m_parryState != ParryState::Ready) return false;

	m_parryState	= ParryState::ParryAccept;
	m_timer			= 0.0f;

	return true;
}

//===========================================================
// 移動入力の更新
//===========================================================
void ParrySystem::Update(float deltaTime)
{
	if (m_parryState == ParryState::Ready) return;

	m_timer += deltaTime;

	// パリィ受付時間が終了した
	if (m_parryState == ParryState::ParryAccept)
	{
		if (m_timer >= kActiveTime)
		{
			m_parryState = ParryState::ParryRecovery;
			m_timer = 0.0f;
		}

		return;
	}

	// リカバリー時間が終了した
	if (m_parryState == ParryState::ParryRecovery)
	{
		if (m_timer >= kRecoveryTime)
		{
			m_parryState = ParryState::Ready;
			m_timer = 0.0f;
		}
	}
}

//===========================================================
// 移動入力の更新
//===========================================================
void ParrySystem::Success()
{
	if (m_parryState != ParryState::ParryAccept) return;

	m_parryState	= ParryState::Ready;
	m_timer			= 0.0f;
}

//===========================================================
// 移動入力の更新
//===========================================================
void ParrySystem::Cancel()
{
	m_parryState	= ParryState::Ready;
	m_timer			= 0.0f;
}
