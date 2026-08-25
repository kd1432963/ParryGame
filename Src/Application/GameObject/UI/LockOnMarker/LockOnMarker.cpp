#include "LockOnMarker.h"

#include "../../Camera/CameraManager.h"
#include "../../Camera/TPSCamera/TPSCamera.h"
#include "../../../Combat/ILockOnTarget.h"
#include "../../../main.h"
#include "../../../Combat/IParryable.h"

namespace
{
	constexpr float	kAppearDuration			= 0.12f;
	constexpr float	kAppearStartScale		= 1.35f;
	constexpr float	kPulseCyclesPerSecond	= 2.5f;
	constexpr float	kPulseAmplitude			= 0.04f;

	constexpr int	kBracketRadius			= 52;
	constexpr int	kBracketLength			= 18;
	constexpr int	kBracketThickness		= 2;

	constexpr int	kDurabilityTotalWidth	= 129;
	constexpr int	kDurabilityHalfHeight	= 7;
	constexpr int	kDurabilityFrameSize	= 3;
	constexpr int	kDurabilityTopMargin	= 20;

	// ロックオンマーカーの角を描画する
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

	// パリィ耐久度を描画する
	void DrawParryDurability(
		KdSpriteShader& spriteShader,
		int				centerX,
		int				markerTopY,
		int				currentDurability,
		int				maxDurability,
		float			alpha
	)
	{
		if (maxDurability <= 0) return;

		currentDurability = std::clamp(currentDurability, 0, maxDurability);

		const int	halfWidth		= kDurabilityTotalWidth / 2;
		const int	barLeft			= centerX - halfWidth;
		const int	barCenterY		= markerTopY + kDurabilityTopMargin;
		const int	barBottomY		= barCenterY - kDurabilityHalfHeight - kDurabilityFrameSize;
		const int	connectorHeight = std::max(1, barBottomY - markerTopY);
		const int	connectorY		= markerTopY + connectorHeight / 2;

		const Math::Color shadowColor(0.00f, 0.00f, 0.00f, 0.55f * alpha);
		const Math::Color frameColor(0.03f, 0.03f, 0.04f, 0.95f * alpha);
		const Math::Color activeColor(1.00f, 0.52f, 0.05f, 1.00f * alpha);
		const Math::Color highlightColor(1.00f, 0.90f, 0.30f, 0.90f * alpha);
		const Math::Color lostColor(0.18f, 0.05f, 0.03f, 0.90f * alpha);

		// ロックオン枠と耐久バーを中央の線で繋げる
		spriteShader.DrawBox(
			centerX,
			connectorY,
			1,
			connectorHeight / 2,
			&activeColor
		);

		// バー全体の影と共通フレーム
		spriteShader.DrawBox(
			centerX + 2,
			barCenterY - 2,
			halfWidth + kDurabilityFrameSize,
			kDurabilityHalfHeight + kDurabilityFrameSize,
			&shadowColor
		);

		spriteShader.DrawBox(
			centerX,
			barCenterY,
			halfWidth + kDurabilityFrameSize,
			kDurabilityHalfHeight + kDurabilityFrameSize,
			&frameColor
		);

		for (int i = 0; i < maxDurability; ++i)
		{
			// 固定幅を最大耐久値で等分する
			const int segmentLeft =
				barLeft +
				kDurabilityTotalWidth * i / maxDurability;

			const int segmentRight =
				barLeft +
				kDurabilityTotalWidth * (i + 1) / maxDurability;

			const int segmentX = (segmentLeft + segmentRight) / 2;
			const int segmentHalfWidth = (segmentRight - segmentLeft) / 2;

			const Math::Color& fillColor =
				i < currentDurability ? activeColor : lostColor;

			spriteShader.DrawBox(
				segmentX,
				barCenterY,
				segmentHalfWidth,
				kDurabilityHalfHeight,
				&fillColor
			);

			if (i < currentDurability)
			{
				spriteShader.DrawBox(
					segmentX,
					barCenterY + kDurabilityHalfHeight - 2,
					segmentHalfWidth - 2,
					1,
					&highlightColor
				);
			}
		}

		for (int i = 1; i < maxDurability; ++i)
		{
			const int separatorX =
				barLeft +
				kDurabilityTotalWidth * i / maxDurability;

			spriteShader.DrawBox(
				separatorX,
				barCenterY,
				1,
				kDurabilityHalfHeight,
				&frameColor
			);
		}
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

	const auto parryable = std::dynamic_pointer_cast<IParryable>(target);

	if (parryable)
	{
		m_currentParryDurability = parryable->GetCurrentParryDurability();
		m_maxParryDurability = parryable->GetMaxParryDurability();
	}
	else
	{
		m_currentParryDurability = 0;
		m_maxParryDurability = 0;
	}

	Math::Vector3 screenPosition = Math::Vector3::Zero;

	tpsCamera->GetCamera()->ConvertWorldToScreenDetail(
		target->GetLockOnMarkerPosition(),
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

	const float deltaTime	= Application::Instance().GetUnscaledDeltaTime();

	m_screenPosition		= { screenPosition.x, screenPosition.y };
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

	const float smoothAppear	= m_appearRate * m_appearRate * (3.0f - 2.0f * m_appearRate);
	const float appearScale		= kAppearStartScale + (1.0f - kAppearStartScale) * smoothAppear;
	const float pulse			= 0.5f + 0.5f * sinf(
			m_visibleTime *
			DirectX::XM_2PI *
			kPulseCyclesPerSecond
		);
	const float pulseScale		= 1.0f + (pulse * 2.0f - 1.0f) * kPulseAmplitude;

	const int	centerX			= static_cast<int>(m_screenPosition.x);
	const int	centerY			= static_cast<int>(m_screenPosition.y);
	const int	bracketRadius	= static_cast<int>(
		kBracketRadius * appearScale * pulseScale
		);
	const Math::Color shadowColor(0.0f,0.0f,0.0f,0.55f * smoothAppear);
	const Math::Color markerColor(1.0f, 0.65f + pulse * 0.15f, 0.10f, 0.96f * smoothAppear);

	auto& spriteShader = KdShaderManager::Instance().m_spriteShader;

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

	DrawParryDurability(
		spriteShader,
		centerX,
		centerY + bracketRadius + kDurabilityTopMargin,
		m_currentParryDurability,
		m_maxParryDurability,
		smoothAppear
	);
}

//===========================================================
// 非表示化
//===========================================================
void LockOnMarker::Hide()
{
	m_wpDisplayedTarget.reset();
	m_screenPosition			= Math::Vector2::Zero;
	m_appearRate				= 0.0f;
	m_visibleTime				= 0.0f;
	m_isVisible					= false;
	m_currentParryDurability	= 0;
	m_maxParryDurability		= 0;
}