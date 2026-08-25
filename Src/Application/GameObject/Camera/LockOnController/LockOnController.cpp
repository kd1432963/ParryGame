#include "LockOnController.h"

#include "../CameraManager.h"
#include "../TPSCamera/TPSCamera.h"
#include "../../Player/Player.h"
#include "../../../Combat/ILockOnTarget.h"
#include "../../../Combat/LockOn/LockOnTargetManager.h"
#include "../../../Scene/SceneManager.h"
#include "../../../main.h"

//===========================================================
// ロックオン入力の更新
//===========================================================
void LockOnController::PreUpdate()
{
	// 右クリックは押した瞬間だけ反応させる
	const bool	isLockOnKeyDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	const bool	isLockOnPressed = isLockOnKeyDown && !m_wasLockOnKeyDown;
	const int	mouseWheelValue = Application::Instance().GetMouseWheelValue();

	m_wasLockOnKeyDown = isLockOnKeyDown;

	// 入力がないフレームは検索処理を行わない
	if (!isLockOnPressed && mouseWheelValue == 0) return;

	const auto	cameraManager = m_wpCameraManager.lock();
	const auto	targetManager = m_wpLockOnTargetManager.lock();

	if (!cameraManager || !targetManager) return;

	// TPSカメラ使用中だけロックオン操作を受け付ける
	if (!std::dynamic_pointer_cast<TPSCamera>(
		cameraManager->GetActiveCamera()
	))
	{
		return;
	}

	// 右クリックでロックオンと解除を切り替える
	if (isLockOnPressed)
	{
		if (targetManager->HasTarget())
		{
			targetManager->ClearTarget();
			return;
		}

		const auto target = FindClosestTargetOnScreen();

		if (target) targetManager->SetTarget(target);

		return;
	}

	// ホイール操作はロックオン中だけ使用する
	const auto currentTarget = targetManager->GetTarget();

	if (!currentTarget) return;

	// ホイール上は左、ホイール下は右
	const int direction = mouseWheelValue > 0 ? -1 : 1;
	const auto nextTarget = FindAdjacentTargetOnScreen(currentTarget, direction);

	if (nextTarget) targetManager->SetTarget(nextTarget);
}

//===========================================================
// 画面内のロックオン候補を取得する
//===========================================================
std::vector<LockOnController::TargetCandidate>
LockOnController::FindTargetsOnScreen() const
{
	const auto cameraManager = m_wpCameraManager.lock();

	if (!cameraManager) return {};

	const auto activeCamera = cameraManager->GetActiveCamera();

	if (!activeCamera || !activeCamera->GetCamera()) return {};

	// 画面中央を原点とした表示可能範囲を求める
	Math::Viewport viewport;
	KdDirect3D::Instance().CopyViewportInfo(viewport);

	const float	halfWidth = viewport.width * 0.5f;
	const float	halfHeight = viewport.height * 0.5f;

	std::vector<TargetCandidate> candidates;

	for (const auto& object : SceneManager::Instance().GetObjList())
	{
		// ILockOnTarget を実装したオブジェクトだけを候補にする
		const auto target = std::dynamic_pointer_cast<ILockOnTarget>(object);

		if (!target || !target->CanLockOn()) continue;

		// 対象の3D座標を画面上の2D座標へ変換する
		Math::Vector3 screenPosition = Math::Vector3::Zero;
		activeCamera->GetCamera()->ConvertWorldToScreenDetail(
			target->GetLockOnPosition(),
			screenPosition
		);

		// カメラより前方かつ画面の幅・高さに収まっているか
		const bool isOnScreen =
			screenPosition.z > 0.0f &&
			std::abs(screenPosition.x) <= halfWidth &&
			std::abs(screenPosition.y) <= halfHeight;

		if (!isOnScreen) continue;

		candidates.push_back({ target, screenPosition.x });
	}

	// ホイールで左右へ移動できるようX座標順に並べる
	std::sort(
		candidates.begin(),
		candidates.end(),
		[](const TargetCandidate& left, const TargetCandidate& right)
		{
			return left.screenX < right.screenX;
		}
	);

	return candidates;
}

//===========================================================
// プレイヤーに最も近い画面内対象を探す
//===========================================================
std::shared_ptr<ILockOnTarget>
LockOnController::FindClosestTargetOnScreen() const
{
	const auto player = m_wpPlayer.lock();

	if (!player) return nullptr;

	const auto candidates = FindTargetsOnScreen();

	float closestDistanceSq = FLT_MAX;
	std::shared_ptr<ILockOnTarget> closestTarget = nullptr;

	for (const auto& candidate : candidates)
	{
		// 距離比較だけなので平方根を使わず二乗距離で比較する
		const float distanceSq =
			(candidate.target->GetLockOnPosition() - player->GetPos())
			.LengthSquared();

		if (distanceSq >= closestDistanceSq) continue;

		closestDistanceSq = distanceSq;
		closestTarget = candidate.target;
	}

	return closestTarget;
}

//===========================================================
// 画面上で左右に隣接する対象を探す
//===========================================================
std::shared_ptr<ILockOnTarget>
LockOnController::FindAdjacentTargetOnScreen(
	const std::shared_ptr<ILockOnTarget>& currentTarget,
	int direction
) const
{
	if (!currentTarget || direction == 0) return nullptr;

	const auto candidates = FindTargetsOnScreen();

	if (candidates.size() <= 1) return nullptr;

	// 現在ロックしている対象の位置を候補配列から探す
	const auto currentIterator = std::find_if(
		candidates.begin(),
		candidates.end(),
		[&currentTarget](const TargetCandidate& candidate)
		{
			return candidate.target == currentTarget;
		}
	);

	// 現在の対象が画面外なら、左端の対象から選び直す
	if (currentIterator == candidates.end())
	{
		return candidates.front().target;
	}

	const int	currentIndex = static_cast<int>(
		std::distance(candidates.begin(), currentIterator)
		);
	const int	candidateCount = static_cast<int>(candidates.size());

	// 負数を避けながら配列の端を循環させる
	const int nextIndex =
		(currentIndex + direction + candidateCount) %
		candidateCount;

	return candidates[nextIndex].target;
}