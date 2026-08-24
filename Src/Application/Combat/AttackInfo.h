#pragma once

//===========================================================
// 1回の攻撃で、攻撃側から被弾側へ渡す情報
//===========================================================
struct AttackInfo
{
	int							damage = 0;						// ダメージ
	Math::Vector3				hitPos = Math::Vector3::Zero;	// 被弾位置
	std::weak_ptr<KdGameObject>	attacker;						// 攻撃者
};