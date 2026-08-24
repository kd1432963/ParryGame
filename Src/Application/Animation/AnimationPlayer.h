#pragma once

//===========================================================
// モーションの再生と切り替え時の姿勢補間を担当する
//===========================================================
class AnimationPlayer
{
public:

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	AnimationPlayer();
	~AnimationPlayer() override;

	//===========================================================
	// 公開関数
	//===========================================================

	// 名前を指定してアニメーションを再生する
	void Play(
		KdModelWork& model,
		const std::string& animationName,
		bool				isLoop,
		float				targetDuration	= 0.0f,
		float				blendDuration	= 0.12f,
		bool				forceRestart	= false
	);

	// アニメーションと切り替え補完を進める
	void Update(KdModelWork& model, float deltaTime);

	// 1.0 を基準に、現在のアニメーションの再生速度を変更する
	void SetAnimationSpeed(float playbackSpeed);

	// 現在のモーションが最後まで再生されたかどうかを返す
	bool IsAnimationEnd() const;

private:

	//===========================================================
	// 内部処理
	//===========================================================
	// 検索済みのアニメーションをKdAnimatorへ設定する
	void SetAnimation(
		KdModelWork&							model,
		const std::shared_ptr<KdAnimationData>& animation,
		bool									isLoop,
		float									blendTime
	);

	// 保存した姿勢から新しいモーションの姿勢へ補間する
	void UpdateBlend(KdModelWork& model, float deltaTime);

	//===========================================================
	// 所有・参照するオブジェクト
	//===========================================================
	std::unique_ptr<KdAnimator>	m_upAnimator = nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	std::vector<Math::Matrix>	m_blendStartTransforms;
	std::string					m_currentAnimationName;
	float						m_animationSpeed	= 1.0f;
	float						m_blendTime			= 0.0f;
	float						m_blendElapsed		= 0.0f;
	bool						m_hasAnimation		= false;

};
