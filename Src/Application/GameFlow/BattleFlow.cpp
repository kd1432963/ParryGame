#include "BattleFlow.h"

#include "../GameObject/Enemy/Enemy.h"
#include "../GameObject/Player/Player.h"

//===========================================================
// 戦闘状態を更新する
//===========================================================
void BattleFlow::Update()
{
	if (m_state != BattleState::Playing) return;

	const auto player = m_wpPlayer.lock();

	// Playerが死亡したらゲームオーバー
	if (!player || player->IsDead())
	{
		ChangeState(BattleState::GameOver);
		return;
	}

	// Enemyが登録されるまではクリア判定を行わない
	if (m_wpEnemies.empty()) return;

	bool hasAliveEnemy = false;

	for (const auto& wpEnemy : m_wpEnemies)
	{
		const auto enemy = wpEnemy.lock();

		if (!enemy || enemy->IsDead()) continue;

		hasAliveEnemy = true;
		break;
	}

	// 生存中のEnemyがいなくなったらクリア
	if (!hasAliveEnemy)
	{
		ChangeState(BattleState::Cleared);
	}
}

//===========================================================
// 監視するEnemyを登録する
//===========================================================
void BattleFlow::RegisterEnemy(
	const std::shared_ptr<Enemy>& enemy
)
{
	if (!enemy) return;

	m_wpEnemies.push_back(enemy);
}

//===========================================================
// 戦闘状態を切り替える
//===========================================================
void BattleFlow::ChangeState(BattleState nextState)
{
	if (m_state == nextState) return;

	m_state = nextState;

#ifdef _DEBUG
	if (m_state == BattleState::Cleared)
	{
		KdDebugGUI::Instance().AddLog("Battle Clear\n");
	}
	else if (m_state == BattleState::GameOver)
	{
		KdDebugGUI::Instance().AddLog("Battle Game Over\n");
	}
#endif
}