#include "AnimationPlayer.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
AnimationPlayer::AnimationPlayer()
	: m_upAnimator(std::make_unique<KdAnimator>())
{}
AnimationPlayer::~AnimationPlayer() = default;

//===========================================================
// 名前を指定してアニメーションを再生する
//===========================================================
void AnimationPlayer::Play(
	KdModelWork	&		model, 
	const std::string & animationName, 
	bool				isLoop, 
	float				targetDuration, 
	float				blendTime, 
	bool				forceRestart)
{
	if (animationName.empty()) return;

	if(!forceRestart	&& 
		m_hasAnimation	&&
		m_currentAnimationName == animationName)
	{
		return;
	}

	const auto spAnimation = model.GetAnimation(animationName);

	if (!spAnimation)
	{
#ifdef _DEBUG
		KdDebugGUI::Instance().AddLog(
			"Animation was not found: %s\n",
			animationName.c_str()
		);
#endif
		return;
	}

	m_animationSpeed = 1.0f;

	// 指定時間で再生し終わるように速度を調整する
	if (targetDuration > 0.0f && spAnimation->m_maxLength > 0.0f)
	{
		const float animationDuration = spAnimation->m_maxLength / 60.0f;
		m_animationSpeed = animationDuration / targetDuration;
	}

	SetAnimation(
		model,
		spAnimation,
		isLoop,
		blendTime
	);

	m_currentAnimationName = animationName;

}

//===========================================================
// アニメーションと切り替え補完を進める
//===========================================================
void AnimationPlayer::Update(KdModelWork & model, float deltaTime)
{
	if (!m_upAnimator|| !m_hasAnimation) return;

	const float animationFrameAdvance = deltaTime * 60.0f * m_animationSpeed;
	auto& nodes = model.WorkNodes();

	m_upAnimator->AdvanceTime(nodes, animationFrameAdvance);

	UpdateBlend(model, deltaTime);
}

//===========================================================
// 1.0 を基準に、現在のアニメーションの再生速度を変更する
//===========================================================
void AnimationPlayer::SetAnimationSpeed(float playbackSpeed)
{
	m_animationSpeed = std::max(0.0f, playbackSpeed);
}

//===========================================================
// 現在のモーションが最後まで再生されたかどうかを返す
//===========================================================
bool AnimationPlayer::IsAnimationEnd() const
{
	return !m_upAnimator || m_upAnimator->IsAnimationEnd();
}

//===========================================================
// 検索済みのアニメーションを KdAnimator へ設定する
// 初期再生、ブレンド無しでなければ、現在の姿勢をブレンド開始姿勢として保存する
//===========================================================
void AnimationPlayer::SetAnimation(
	KdModelWork&							model,
	const std::shared_ptr<KdAnimationData>& animation,
	bool									isLoop,
	float									blendTime)
{
	if (!m_upAnimator || !animation) return;

	// 再生中の姿勢を、ブレンド開始姿勢として保存する
	if (m_hasAnimation && blendTime > 0.0f)
	{
		// ブレンド開始姿勢を保存する
		const auto& nodes = model.GetNodes();
		
		m_blendStartTransforms.resize(nodes.size());
		
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			m_blendStartTransforms[i] = nodes[i].m_localTransform;
		}

		// ブレンド時間を設定する
		m_blendTime		= blendTime;
		m_blendElapsed	= 0.0f;
	}
	else
	{
		// 初回再生、または即時切り替えではブレンドしない
		m_blendStartTransforms.clear();

		m_blendTime		= 0.0f;
		m_blendElapsed	= 0.0f;
	}

	// KdAnimatorへ実際にアニメーションを設定する
	m_upAnimator->SetAnimation(animation, isLoop);

	// アニメーション設定済みにする
	m_hasAnimation = true;
}

//===========================================================
// 保存した姿勢から新しいモーションの姿勢へ補間する
//===========================================================
void AnimationPlayer::UpdateBlend(KdModelWork & model, float deltaTime)
{
	if (m_blendTime <= 0.0f) return;

	auto& nodes = model.WorkNodes();

	// モデルが変わってノード数が違う場合は補間を中止する
	if (m_blendStartTransforms.size() != nodes.size())
	{
		m_blendStartTransforms.clear();

		m_blendTime		= 0.0f;
		m_blendElapsed	= 0.0f;

		return;
	}

	m_blendElapsed += std::max(0.0f, deltaTime);

	// ブレンド率を計算する
	float blendRate = std::clamp(
		m_blendElapsed / m_blendTime,
		0.0f,
		1.0f
	);

	// 補間の開始と終了を滑らかにする（スムーズステップ）
	blendRate = blendRate * blendRate * (3.0f - 2.0f * blendRate);

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		Math::Vector3		startScale;
		Math::Quaternion	startRotation;
		Math::Vector3		startTranslation;

		Math::Vector3		targetScale;
		Math::Quaternion	targetRotation;
		Math::Vector3		targetTranslation;

		// 行列を大きさ・回転・位置へ分解する
		const bool canDecomposeStart = m_blendStartTransforms[i].Decompose(
			startScale,
			startRotation,
			startTranslation
		);

		const bool canDecomposeTarget = nodes[i].m_localTransform.Decompose(
			targetScale,
			targetRotation,
			targetTranslation
		);

		if (!canDecomposeStart || !canDecomposeTarget) continue;

		// 大きさと位置は直線補間する
		const Math::Vector3 scale = DirectX::XMVectorLerp(
			startScale,
			targetScale,
			blendRate
		);

		const Math::Vector3 translation = DirectX::XMVectorLerp(
			startTranslation,
			targetTranslation,
			blendRate
		);

		// Quaternion を成分ごとに直線補間すると単位長を保てず、
		// 不自然な回転になる可能性があるため球面線形補間する
		const Math::Quaternion rotation = DirectX::XMQuaternionSlerp(
			startRotation,
			targetRotation,
			blendRate
		);

		// 補間した値からノード行列を作り直す
		nodes[i].m_localTransform = Math::Matrix::CreateScale(scale) *
			Math::Matrix::CreateFromQuaternion(rotation) *
			Math::Matrix::CreateTranslation(translation);
	}

	// 指定時間まで補間したらブレンドを終了する
	if (m_blendElapsed >= m_blendTime)
	{
		m_blendStartTransforms.clear();

		m_blendTime		= 0.0f;
		m_blendElapsed	= 0.0f;
	}
}
