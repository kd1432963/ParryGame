#include "CharacterSeparation.h"

namespace
{
	// 完全に同じ座標にいる場合のゼロ除算を防ぐ
	constexpr float	kMinDistanceSquared = 0.000001f;

	// 多数のキャラが密集した場合に、残った重なりを再補正する回数
	constexpr int	kResolveIterations = 2;

	//===========================================================
	// 2つの円柱が重なっている場合に半分ずつ押し戻す
	//===========================================================
	void ResolvePair(
		KdGameObject&	firstObject,
		float			firstRadius,
		float			firstHeight,
		KdGameObject&	secondObject,
		float			secondRadius,
		float			secondHeight
	)
	{
		Math::Vector3	firstPosition	= firstObject.GetPos();
		Math::Vector3	secondPosition	= secondObject.GetPos();

		//===========================================================
		// 上下方向の重なりを確認
		//===========================================================
		const float	firstBottom		= firstPosition.y;
		const float	firstTop		= firstBottom + firstHeight;
		const float	secondBottom	= secondPosition.y;
		const float	secondTop		= secondBottom + secondHeight;

		if (firstTop <= secondBottom || secondTop <= firstBottom) return;

		//===========================================================
		// XZ平面上の円同士の重なりを確認
		//===========================================================
		Math::Vector3 toSecond = secondPosition - firstPosition;
		toSecond.y = 0.0f;

		const float	minDistance		= firstRadius + secondRadius;
		const float	distanceSquared = toSecond.LengthSquared();

		if (distanceSquared >= minDistance * minDistance) return;

		float			distance		= 0.0f;
		Math::Vector3	pushDirection	= { 1.0f, 0.0f, 0.0f };

		if (distanceSquared > kMinDistanceSquared)
		{
			distance		= std::sqrt(distanceSquared);
			pushDirection	= toSecond / distance;
		}

		// 重なった距離を両者へ半分ずつ分配する
		const float			overlapDistance = minDistance - distance;
		const Math::Vector3	correction		= pushDirection * (overlapDistance * 0.5f);

		firstPosition	-= correction;
		secondPosition	+= correction;

		firstObject.SetPos(firstPosition);
		secondObject.SetPos(secondPosition);
	}
}

//===========================================================
// 押し戻し対象を登録する
//===========================================================
void CharacterSeparation::RegisterBody(
	const std::shared_ptr<KdGameObject>& object,
	float radius,
	float height
)
{
	if (!object || radius <= 0.0f || height <= 0.0f) return;

	// 同じオブジェクトの二重登録を防ぐ
	for (const auto& body : m_bodies)
	{
		if (body.object.lock() == object) return;
	}

	m_bodies.push_back({ object, radius, height });
}

//===========================================================
// 全キャラクターの移動後に重なりを解消する
//===========================================================
void CharacterSeparation::PostUpdate()
{
	// 消滅済みのキャラクターを登録一覧から除外する
	m_bodies.erase(
		std::remove_if(
			m_bodies.begin(),
			m_bodies.end(),
			[](const BodyEntry& body)
			{
				const auto object = body.object.lock();
				return !object || object->IsExpired();
			}
		),
		m_bodies.end()
	);

	// 密集時に押し戻しが残らないよう数回補正する
	for (int iteration = 0; iteration < kResolveIterations; ++iteration)
	{
		for (size_t firstIndex = 0; firstIndex < m_bodies.size(); ++firstIndex)
		{
			const auto firstObject = m_bodies[firstIndex].object.lock();
			if (!firstObject) continue;

			for (size_t secondIndex = firstIndex + 1;
				secondIndex < m_bodies.size();
				++secondIndex)
			{
				const auto secondObject = m_bodies[secondIndex].object.lock();
				if (!secondObject) continue;

				ResolvePair(
					*firstObject,
					m_bodies[firstIndex].radius,
					m_bodies[firstIndex].height,
					*secondObject,
					m_bodies[secondIndex].radius,
					m_bodies[secondIndex].height
				);
			}
		}
	}
}