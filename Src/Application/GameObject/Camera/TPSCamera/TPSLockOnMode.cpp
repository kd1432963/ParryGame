#include "TPSLockOnMode.h"

#include "../../../Combat/ILockOnTarget.h"

//===========================================================
// 
//===========================================================
bool TPSLockOnMode::SetTarget(
	const std::shared_ptr<ILockOnTarget>& target
)
{
	if (!target || !target->CanLockOn()) return false;

	m_wpTarget = target;

	return true;
}

//===========================================================
// ターゲットを解除する
//===========================================================
void TPSLockOnMode::ClearTarget()
{
	m_wpTarget.reset();
}

//===========================================================
// 現在有効なターゲットがあるか
//===========================================================
bool TPSLockOnMode::HasTarget() const
{
	const auto target = m_wpTarget.lock();

	return target && target->CanLockOn();
}

//===========================================================
// 現在のターゲットを返す
//===========================================================
std::shared_ptr<ILockOnTarget> TPSLockOnMode::GetTarget() const
{
	return m_wpTarget.lock();
}

//===========================================================
// ロックオン時に必要なカメラ情報を計算する
//===========================================================
bool TPSLockOnMode::Update(
	const Math::Vector3& followPosition,		// プレイヤーの位置
	float					baseDistance,		// カメラの基本距離
	float					focusHeight,		// プレイヤーの注視点の高さ
	float					unscaledDeltaTime,	// ヒットストップの影響を受けない実時間
	Math::Vector3&			cameraAngle,		// カメラの角度	
	Math::Vector3&			outFocusPosition,	// カメラの注視点
	float&					outDistance			// カメラの距離
) const
{
	const auto target = m_wpTarget.lock();

	if (!target || !target->CanLockOn()) return false;

	const Math::Vector3 targetPosition = target->GetLockOnPosition();

	Math::Vector3 toTarget = targetPosition - followPosition;
	toTarget.y = 0.0f;

	if (toTarget.LengthSquared() <= 0.0001f) return false;

	const float separation = toTarget.Length();

	// プレイヤーの胸元を通常時の注視点にする
	Math::Vector3 playerFocusPosition = followPosition;
	playerFocusPosition.y += focusHeight;

	// プレイヤーと敵の間を注視する
	outFocusPosition = playerFocusPosition +
		(targetPosition - playerFocusPosition) * m_focusBias;

	// 両者が離れるほどカメラも後ろへ引く
	outDistance = std::clamp(
		baseDistance + separation * m_distancePerSeparation,
		baseDistance,
		m_maxDistance
	);

	// 現在角度から敵方向へ滑らかに回転する
	const float	targetYaw = atan2f(toTarget.x, toTarget.z);
	const float	currentYaw = DirectX::XMConvertToRadians(cameraAngle.y);
	const float	angleDifference = DirectX::XMScalarModAngle(
		targetYaw - currentYaw
	);

	const float rotationRate = 1.0f - expf(
		-m_rotationSharpness * unscaledDeltaTime
	);

	cameraAngle.y = DirectX::XMConvertToDegrees(
		currentYaw + angleDifference * rotationRate
	);

	return true;
}