#include "AttackHitBox.h"

#include "AttackInfo.h"
#include "../Scene/SceneManager.h"
#include "../GameObject/Player/Player.h"

AttackHitBox::AttackHitBox(
	const Math::Vector3&	pos,
	float					radius,
	const AttackInfo&		attackInfo
)
	: m_pos(pos),
	m_radius(radius),
	m_upAttackInfo(std::make_unique<AttackInfo>(attackInfo))
{
#ifdef _DEBUG
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
#endif
}
AttackHitBox::~AttackHitBox() = default;

void AttackHitBox::PostUpdate()
{
	//===========================================================
	// 今回の攻撃範囲を球として作る
	//===========================================================
	KdCollider::SphereInfo attackSphere(
		KdCollider::TypeDamage,
		m_pos,
		m_radius
	);

#ifdef _DEBUG
	if (m_pDebugWire)
	{
		m_pDebugWire->AddDebugSphere(
			m_pos,
			m_radius,
			Math::Color(1.0f, 0.0f, 0.0f, 1.0f)
		);
	}
#endif

	// ===========================================================
	// 攻撃判定の当たり判定を行う
	const auto& objectList = SceneManager::Instance().GetObjList();
	for (const auto& object : objectList)
	{
		if (!object) continue;

		const auto player = std::dynamic_pointer_cast<Player>(object);

		if (!player) continue;

		std::list<KdCollider::CollisionResult> results;

		if (!player->Intersects(attackSphere, &results)) continue;

		const KdCollider::CollisionResult* bestResult = nullptr;

		// 複数の被弾判定に触れた場合は、最も深く重なった接触点を採用する
		for (const auto& result : results)
		{
			if (bestResult &&
				result.m_overlapDistance <= bestResult->m_overlapDistance)
			{
				continue;
			}

			bestResult = &result;
		}

		// 詳細結果が取れない場合でも、攻撃判定の中心を予備座標として渡す
		m_upAttackInfo->hitPos = bestResult ? bestResult->m_hitPos : m_pos;

		player->OnHit(*m_upAttackInfo);
		break;
	}

	// 1回判定したら、次フレームのPreUpdateで削除される
	--m_life;

	if (m_life <= 0)
	{
		m_isExpired = true;
	}
}
