#include "TPSCamera.h"

#include "TPSFreeLookMode.h"
#include "TPSLockOnMode.h"
#include "../../../main.h"
#include "../../../Combat/ILockOnTarget.h"
#include "../../../Combat/LockOn/LockOnTargetManager.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
TPSCamera::TPSCamera()
	: m_upFreeLookMode(std::make_unique<TPSFreeLookMode>()),
	  m_upLockOnMode(std::make_unique<TPSLockOnMode>())
{}
TPSCamera::~TPSCamera() = default;

//===========================================================
// 初期化関数
//===========================================================
void TPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	m_currentDistance = m_baseDistance;
	m_mLocalPos = Math::Matrix::CreateTranslation(
		0.5f,
		0.0f,
		-m_currentDistance
	);
}

//===========================================================
// 更新関数
//===========================================================
void TPSCamera::PostUpdate()
{
	const auto _spTarget = m_wpTarget.lock();

	if (!_spTarget) return;


	const auto targetManager = m_wpLockOnTargetManager.lock();
	const auto lockOnTarget = targetManager ? targetManager->GetTarget() : nullptr;

	// Managerに対象が設定された瞬間にロックオンモードへ切り替える
	if (lockOnTarget && m_mode != TPSCameraModeId::LockOn)
	{
		m_mode = TPSCameraModeId::LockOn;

		if (m_upFreeLookMode)
		{
			m_upFreeLookMode->ResetInput();
		}
	}

	const float unscaledDeltaTime =	Application::Instance().GetUnscaledDeltaTime();

	Math::Vector3 desiredFocusPosition = _spTarget->GetPos();
	desiredFocusPosition.y += m_focusHeight;

	float	desiredDistance = m_baseDistance;
	bool	isLockOnActive = false;

	if (m_mode == TPSCameraModeId::LockOn &&
		m_upLockOnMode)
	{
		isLockOnActive = m_upLockOnMode->Update(
			lockOnTarget,
			_spTarget->GetPos(),
			m_baseDistance,
			m_focusHeight,
			unscaledDeltaTime,
			m_DegAng,
			desiredFocusPosition,
			desiredDistance
		);
	}

	// 対象が倒れた場合などは通常視点へ戻す
	if (!isLockOnActive)
	{
		if (m_mode == TPSCameraModeId::LockOn)
		{
			ClearLockOnTarget();
		}

		if (m_upFreeLookMode)
		{
			m_upFreeLookMode->Update(m_DegAng);
		}
	}

	// 初回だけ補間元を現在の目標値へ合わせる
	if (!m_hasCameraState)
	{
		m_focusPosition = desiredFocusPosition;
		m_currentDistance = desiredDistance;
		m_hasCameraState = true;
	}

	const float focusRate = 1.0f - expf(
		-m_focusSharpness * unscaledDeltaTime
	);

	const float distanceRate = 1.0f - expf(
		-m_distanceSharpness * unscaledDeltaTime
	);

	m_focusPosition +=
		(desiredFocusPosition - m_focusPosition) * focusRate;

	m_currentDistance +=
		(desiredDistance - m_currentDistance) * distanceRate;

	m_mRotation = GetRotationMatrix();

	m_mLocalPos = Math::Matrix::CreateTranslation(
		0.5f,
		0.0f,
		-m_currentDistance
	);

	const Math::Matrix targetMatrix =
		Math::Matrix::CreateTranslation(m_focusPosition);

	m_mWorld =
		m_mLocalPos *
		m_mRotation *
		targetMatrix;

	// ↓めり込み防止の為の座標補正計算↓
	// ①当たり判定(レイ判定)用の情報作成
	KdCollider::RayInfo rayInfo;
	// レイの発射位置を設定
	rayInfo.m_pos = GetPos();

	// レイの発射方向を設定
	rayInfo.m_dir = Math::Vector3::Down;
	// レイの長さを設定
	rayInfo.m_range = 1000.f;
	if (_spTarget)
	{
		Math::Vector3 _targetPos = _spTarget->GetPos();
		_targetPos.y	+= 0.1f;
		rayInfo.m_dir	= _targetPos - GetPos();
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = KdCollider::TypeGround;

	// ②HIT判定対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// ③ 結果を使って座標を補完する
			// レイに当たったリストから一番近いオブジェクトを検出
			float maxOverLap = 0;
			Math::Vector3 hitPos = {};
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// レイを遮断しオーバーした長さが
				// 一番長いものを探す
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			if (hit)
			{
				// 何かしらの障害物に当たっている
				Math::Vector3 _hitPos = hitPos;
				_hitPos += rayInfo.m_dir * 0.4f;
				SetPos(_hitPos);
			}
		}
	}
}

//===========================================================
// カメラ切り替え時の処理
//===========================================================
void TPSCamera::OnActivated()
{
	CameraBase::OnActivated();

	if (!m_upFreeLookMode) return;

	m_upFreeLookMode->ResetInput();
}

//===========================================================
// ロックオン対象の設定・解除
//===========================================================
//===========================================================
// ロックオン対象を設定する
//===========================================================
bool TPSCamera::SetLockOnTarget(
	const std::shared_ptr<ILockOnTarget>& target
)
{
	const auto targetManager = m_wpLockOnTargetManager.lock();

	if (!targetManager ||
		!targetManager->SetTarget(target))
	{
		return false;
	}

	m_mode = TPSCameraModeId::LockOn;

	if (m_upFreeLookMode)
	{
		m_upFreeLookMode->ResetInput();
	}

	return true;
}

//===========================================================
// ロックオン対象を解除する
//===========================================================
void TPSCamera::ClearLockOnTarget()
{
	const auto targetManager = m_wpLockOnTargetManager.lock();

	if (targetManager)
	{
		targetManager->ClearTarget();
	}

	m_mode = TPSCameraModeId::FreeLook;

	if (m_upFreeLookMode)
	{
		m_upFreeLookMode->ResetInput();
	}
}

//===========================================================
// ロックオン対象が存在するか
//===========================================================
bool TPSCamera::HasLockOnTarget() const
{
	const auto targetManager = m_wpLockOnTargetManager.lock();

	return targetManager &&
		targetManager->HasTarget();
}

//===========================================================
// 現在のロックオン対象を取得する
//===========================================================
std::shared_ptr<ILockOnTarget>TPSCamera::GetLockOnTarget() const
{
	const auto targetManager = m_wpLockOnTargetManager.lock();

	if (!targetManager) return nullptr;

	return targetManager->GetTarget();
}