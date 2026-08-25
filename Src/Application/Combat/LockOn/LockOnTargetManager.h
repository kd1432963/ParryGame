#pragma once

class ILockOnTarget;

//===========================================================
// 現在のロックオン対象を共有・管理するクラス
//===========================================================
class LockOnTargetManager
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	LockOnTargetManager()	= default;
	~LockOnTargetManager()	= default;

	//===========================================================
	// 公開関数
	//===========================================================
	bool SetTarget(const std::shared_ptr<ILockOnTarget>& target);
	void ClearTarget();

	bool HasTarget() const;
	std::shared_ptr<ILockOnTarget> GetTarget() const;

private:

	//===========================================================
	// 所有するロックオン対象
	//===========================================================
	std::weak_ptr<ILockOnTarget> m_wpTarget;
};