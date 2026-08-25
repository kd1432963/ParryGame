#include "LockOnController.h"

#include "../CameraManager.h"
#include "../TPSCamera/TPSCamera.h"
#include "../../Player/Player.h"
#include "../../../Combat/ILockOnTarget.h"
#include "../../../Scene/SceneManager.h"

void LockOnController::PreUpdate()
{
	const bool	isLockOnKeyDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	const bool	isLockOnPressed = isLockOnKeyDown && !m_wasLockOnKeyDown;

	m_wasLockOnKeyDown = isLockOnKeyDown;

	if (!isLockOnPressed) return;

	const auto cameraManager = m_wpCameraManager.lock();

	if (!cameraManager) return;

	const auto tpsCamera = std::dynamic_pointer_cast<TPSCamera>(
		cameraManager->GetActiveCamera()
	);

	// TPSカメラ以外を使用中なら操作を受け付けない
	if (!tpsCamera) return;

	// ロックオン中に押した場合は解除する
	if (tpsCamera->HasLockOnTarget())
	{
		tpsCamera->ClearLockOnTarget();
		return;
	}

	const auto target = FindClosestTargetOnScreen();

	if (!target) return;

	tpsCamera->SetLockOnTarget(target);
}

std::shared_ptr<ILockOnTarget>
LockOnController::FindClosestTargetOnScreen() const
{
	const auto	cameraManager = m_wpCameraManager.lock();
	const auto	player = m_wpPlayer.lock();

	if (!cameraManager || !player) return nullptr;

	const auto activeCamera = cameraManager->GetActiveCamera();

	if (!activeCamera || !activeCamera->GetCamera()) return nullptr;

	Math::Viewport viewport;
	KdDirect3D::Instance().CopyViewportInfo(viewport);

	const float	halfWidth			= viewport.width * 0.5f;
	const float	halfHeight			= viewport.height * 0.5f;
	float		closestDistanceSq	= FLT_MAX;

	std::shared_ptr<ILockOnTarget> closestTarget = nullptr;

	for (const auto& object : SceneManager::Instance().GetObjList())
	{
		const auto target = std::dynamic_pointer_cast<ILockOnTarget>(object);

		if (!target || !target->CanLockOn()) continue;

		Math::Vector3 screenPosition = Math::Vector3::Zero;

		activeCamera->GetCamera()->ConvertWorldToScreenDetail(
			target->GetLockOnPosition(),
			screenPosition
		);

		const bool isOnScreen =
			screenPosition.z > 0.0f &&
			std::abs(screenPosition.x) <= halfWidth &&
			std::abs(screenPosition.y) <= halfHeight;

		if (!isOnScreen) continue;

		const float distanceSq = (
			target->GetLockOnPosition() - player->GetPos()
			).LengthSquared();

		if (distanceSq >= closestDistanceSq) continue;

		closestDistanceSq = distanceSq;
		closestTarget = target;
	}

	return closestTarget;
}