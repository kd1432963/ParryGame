#include "TPSLockOnMode.h"

#include "../../../Combat/ILockOnTarget.h"

//===========================================================
// ロックオン時に必要なカメラ情報を計算する
//===========================================================
bool TPSLockOnMode::Update(
	const std::shared_ptr<ILockOnTarget>&	target,
	const Math::Vector3&					followPosition,
	float									baseDistance,
	float									focusHeight,
	float									unscaledDeltaTime,
	Math::Vector3&							cameraAngle,
	Math::Vector3&							outFocusPosition,
	float&									outDistance
) const
{
	if (!target || !target->CanLockOn()) return false;

	const Math::Vector3 targetPosition = target->GetLockOnPosition();

	// プレイヤーから敵への水平方向を求める
	Math::Vector3 toTarget = targetPosition - followPosition;
	toTarget.y = 0.0f;

	if (toTarget.LengthSquared() <= 0.0001f) return false;

	const float separation = toTarget.Length();

	// プレイヤーの胸元を通常時の注視点にする
	Math::Vector3 playerFocusPosition = followPosition;
	playerFocusPosition.y += focusHeight;

	// プレイヤーと敵の間へ注視点を移動する
	outFocusPosition = playerFocusPosition +
		(targetPosition - playerFocusPosition) * m_focusBias;

	// プレイヤーと敵が離れるほどカメラを後ろへ引く
	outDistance = std::clamp(
		baseDistance + separation * m_distancePerSeparation,
		baseDistance,
		m_maxDistance
	);

	// プレイヤーから見た敵の方向を目標角度にする
	const float	targetYaw		= atan2f(toTarget.x, toTarget.z);
	const float	currentYaw		= DirectX::XMConvertToRadians(cameraAngle.y);
	const float	angleDifference = DirectX::XMScalarModAngle(targetYaw - currentYaw);

	// フレームレートに依存しにくい割合で目標角度へ近づける
	const float rotationRate = 1.0f - expf(
		-m_rotationSharpness * unscaledDeltaTime
	);

	cameraAngle.y = DirectX::XMConvertToDegrees(
		currentYaw + angleDifference * rotationRate
	);

	return true;
}