#include "LockOnMarker.h"

#include "../../Camera/CameraManager.h"
#include "../../Camera/TPSCamera/TPSCamera.h"
#include "../../../Combat/ILockOnTarget.h"
#include "../../../main.h"

namespace
{
	constexpr float	kAppearDuration			= 0.12f;
	constexpr float	kAppearStartScale		= 1.35f;
	constexpr float	kPulseCyclesPerSecond	= 2.5f;
	constexpr float	kPulseAmplitude			= 0.04f;

	constexpr int	kBracketRadius			= 52;
	constexpr int	kBracketLength			= 18;
	constexpr int	kBracketThickness		= 2;

	void DrawLockOnCorners(
		KdSpriteShader&		spriteShader,
		int					centerX,
		int					centerY,
		int					radius,
		const Math::Color&	color
	)
	{
		const int	left		= centerX - radius;
		const int	right		= centerX + radius;
		const int	top			= centerY + radius;
		const int	bottom		= centerY - radius;
		const int	halfLength	= kBracketLength / 2;

		spriteShader.DrawBox(left + halfLength, top, halfLength, kBracketThickness, &color);
		spriteShader.DrawBox(left, top - halfLength, kBracketThickness, halfLength, &color);

		spriteShader.DrawBox(right - halfLength, top, halfLength, kBracketThickness, &color);
		spriteShader.DrawBox(right, top - halfLength, kBracketThickness, halfLength, &color);

		spriteShader.DrawBox(left + halfLength, bottom, halfLength, kBracketThickness, &color);
		spriteShader.DrawBox(left, bottom + halfLength, kBracketThickness, halfLength, &color);

		spriteShader.DrawBox(right - halfLength, bottom, halfLength, kBracketThickness, &color);
		spriteShader.DrawBox(right, bottom + halfLength, kBracketThickness, halfLength, &color);
	}
}

//===========================================================
// 更新関数
//===========================================================
void LockOnMarker::Update()
{
	const auto cameraManager = m_wpCameraManager.lock();

	if (!cameraManager)
	{
		Hide();
		return;
	}

	const auto tpsCamera = std::dynamic_pointer_cast<TPSCamera>(
		cameraManager->GetActiveCamera()
	);

	if (!tpsCamera)
	{
		Hide();
		return;
	}

	const auto target = tpsCamera->GetLockOnTarget();

	if (!target ||
		!target->CanLockOn() ||
		!tpsCamera->GetCamera())
	{
		Hide();
		return;
	}

	if (m_wpDisplayedTarget.lock() != target)
	{
		m_wpDisplayedTarget = target;
		m_appearRate		= 0.0f;
		m_visibleTime		= 0.0f;
	}

	Math::Vector3 screenPosition = Math::Vector3::Zero;

	tpsCamera->GetCamera()->ConvertWorldToScreenDetail(
		target->GetLockOnPosition(),
		screenPosition
	);

	Math::Viewport viewport;
	KdDirect3D::Instance().CopyViewportInfo(viewport);

	const bool isOnScreen =
		screenPosition.z > 0.0f &&
		std::abs(screenPosition.x) <= viewport.width * 0.5f &&
		std::abs(screenPosition.y) <= viewport.height * 0.5f;

	if (!isOnScreen)
	{
		m_isVisible = false;
		return;
	}

	const float deltaTime =
		Application::Instance().GetUnscaledDeltaTime();

	m_screenPosition = { screenPosition.x, screenPosition.y };
	m_appearRate = std::min(
		1.0f,
		m_appearRate + deltaTime / kAppearDuration
	);
	m_visibleTime += deltaTime;
	m_isVisible = true;
}

//===========================================================
// 描画関数
//===========================================================
void LockOnMarker::DrawSprite()
{
	if (!m_isVisible) return;

	const float smoothAppear =
		m_appearRate *
		m_appearRate *
		(3.0f - 2.0f * m_appearRate);

	const float appearScale =
		kAppearStartScale +
		(1.0f - kAppearStartScale) * smoothAppear;

	const float pulse =
		0.5f +
		0.5f * sinf(
			m_visibleTime *
			DirectX::XM_2PI *
			kPulseCyclesPerSecond
		);

	const float pulseScale =
		1.0f +
		(pulse * 2.0f - 1.0f) *
		kPulseAmplitude;

	const int	centerX			= static_cast<int>(m_screenPosition.x);
	const int	centerY			= static_cast<int>(m_screenPosition.y);
	const int	bracketRadius	= static_cast<int>(
		kBracketRadius * appearScale * pulseScale
		);

	const Math::Color shadowColor(
		0.0f,
		0.0f,
		0.0f,
		0.55f * smoothAppear
	);

	const Math::Color markerColor(
		1.0f,
		0.65f + pulse * 0.15f,
		0.10f,
		0.96f * smoothAppear
	);

	auto& spriteShader =
		KdShaderManager::Instance().m_spriteShader;

	spriteShader.Begin();

	DrawLockOnCorners(
		spriteShader,
		centerX + 2,
		centerY - 2,
		bracketRadius,
		shadowColor
	);

	DrawLockOnCorners(
		spriteShader,
		centerX,
		centerY,
		bracketRadius,
		markerColor
	);

	spriteShader.End();
}

//===========================================================
// 非表示化
//===========================================================
void LockOnMarker::Hide()
{
	m_wpDisplayedTarget.reset();
	m_screenPosition	= Math::Vector2::Zero;
	m_appearRate		= 0.0f;
	m_visibleTime		= 0.0f;
	m_isVisible			= false;
}