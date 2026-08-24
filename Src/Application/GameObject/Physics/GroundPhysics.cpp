#include "GroundPhysics.h"

#include "../../Scene/SceneManager.h"

//===========================================================
// 定数
//===========================================================
namespace
{
	// 1秒間に下向きへ加える速度
	constexpr float kGravityAcceleration	= 30.0f;

	// 落下速度が大きくなりすぎないようにする
	constexpr float kTerminalVelocity		= 25.0f;

	// 足裏より少し上からレイを飛ばし、床へのめり込みも拾う
	constexpr float kRayStartHeight			= 0.5f;

	// 小さな段差や数値誤差では床から離れないようにする
	constexpr float kGroundSnapDistance		= 0.15f;

	// 処理落ち直後に一度で大きく落下することを防ぐ
	constexpr float kMaxDeltaTime			= 1.0f / 15.0f;

	// 足元から下向きにレイを飛ばして地面を探す
	bool FindGround(
		const KdGameObject&		gameObject,
		const Math::Vector3&	rayStart,
		float					rayRange,
		Math::Vector3&			outGroundPos
	)
	{
		KdCollider::RayInfo rayInfo(
			KdCollider::TypeGround,
			rayStart,
			Math::Vector3::Down,
			rayRange
		);

#ifdef _DEBUG
		// デバッグ用にレイを描画する
		if (const auto debugWire = gameObject.GetDebugWire())
		{
			debugWire->AddDebugLine(rayInfo.m_pos, rayInfo.m_pos + rayInfo.m_dir * rayInfo.m_range, Math::Vector4(1, 0, 0, 1));
		}
#endif

		float	largestOverlap	= -FLT_MAX;
		bool	foundGround		= false;

		// 現在のシーンにある地面判定へレイを当てる
		for (const auto& sceneObject : SceneManager::Instance().GetObjList())
		{
			if (!sceneObject || sceneObject.get() == &gameObject) continue;

			std::list<KdCollider::CollisionResult> results;

			if (!sceneObject->Intersects(rayInfo, &results)) continue;

			// レイの開始地点から最も近い地面を採用する
			for (const auto& result : results)
			{
				if (result.m_overlapDistance <= largestOverlap)continue;

				largestOverlap	= result.m_overlapDistance;
				outGroundPos	= result.m_hitPos;
				foundGround		= true;
			}
		}

		return foundGround;
	}
}


//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
GroundPhysics::GroundPhysics()	= default;
GroundPhysics::~GroundPhysics() = default;

//===========================================================
// 更新関数
//===========================================================
void GroundPhysics::Update(
	KdGameObject&	gameObject,
	float			deltaTime,
	float			footOffsetY)
{
	if (deltaTime <= 0.0f) return;

	deltaTime = std::max(0.0f, deltaTime);

	//===========================================================
	// 重力による落下位置を計算
	//===========================================================
	Math::Vector3 currentPos = gameObject.GetPos();

	m_verticalVelocity -= kGravityAcceleration * deltaTime;
	m_verticalVelocity = std::max(m_verticalVelocity, -kTerminalVelocity);

	Math::Vector3 nextPos = currentPos;
	nextPos.y += m_verticalVelocity * deltaTime;

	//===========================================================
	// 落下前の足元から、移動量を含めて下向きに地面を探す
	//===========================================================
	const float currentFootY = currentPos.y + footOffsetY;
	const float nextFootY = nextPos.y + footOffsetY;
	const float	fallDistance = std::max(0.0f, currentFootY - nextFootY);

	Math::Vector3	rayStart	= nextPos;
	rayStart.y = currentFootY + kRayStartHeight;

	const float		rayRange	= kRayStartHeight + fallDistance + kGroundSnapDistance;

	Math::Vector3	groundPos	= Math::Vector3::Zero;

	if (m_verticalVelocity <= 0.0f && FindGround(gameObject, rayStart, rayRange, groundPos))
	{
		//===========================================================
		// 地面が見つかった場合は、地面の上に立つように補正する
		//===========================================================
		nextPos.y = groundPos.y - footOffsetY;
		m_verticalVelocity = 0.0f;
		m_isGrounded = true;
	}
	else
	{
		m_isGrounded = false;
	}

	gameObject.SetPos(nextPos);
}
