#include "LockOnTargetManager.h"

#include "../ILockOnTarget.h"

//===========================================================
// ロックオン対象を設定する
//===========================================================
bool LockOnTargetManager::SetTarget(
	const std::shared_ptr<ILockOnTarget>& target
)
{
	if (!target || !target->CanLockOn()) return false;

	m_wpTarget = target;

	return true;
}

//===========================================================
// ロックオン対象を解除する
//===========================================================
void LockOnTargetManager::ClearTarget()
{
	m_wpTarget.reset();
}

//===========================================================
// 有効なロックオン対象が存在するか
//===========================================================
bool LockOnTargetManager::HasTarget() const
{
	return GetTarget() != nullptr;
}

//===========================================================
// 現在のロックオン対象を取得する
//===========================================================
std::shared_ptr<ILockOnTarget>
LockOnTargetManager::GetTarget() const
{
	const auto target = m_wpTarget.lock();

	if (!target || !target->CanLockOn()) return nullptr;

	return target;
}