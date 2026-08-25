#pragma once

//===========================================================
// Enemyへ設定する種類
//===========================================================
enum class EnemyType
{
	Golem,
	ResonanceCaster
};

//===========================================================
// 攻撃判定を作る方法
//===========================================================
enum class EnemyAttackType
{
	MeleeCombo,
	ResonanceProjectile
};

// 1回の攻撃モーションと、判定を発生させる時刻
struct EnemyAttackStepConfig
{
	float	durationSeconds = 1.0f;
	float	hitTimingRate	= 0.5f;
};

//===========================================================
// 敵ごとに変わる数値と素材名を一か所へまとめる
//===========================================================
struct EnemyConfig
{
	std::string modelFileName		= ""; // モデルファイル名
	std::string idleAnimationName	= ""; // 待機アニメーション名
	std::string chaseAnimationName	= ""; // 追跡アニメーション名
	std::string attackAnimationName	= ""; // 攻撃アニメーション名
	std::string hitAnimationName	= ""; // 被弾アニメーション名
	std::string deathAnimationName	= ""; // 死亡アニメーション名
	std::string attackNodeName		= ""; // 攻撃判定を発生させるノード名
	std::string lockOnNodeName		= ""; // ロックオンマーカーを表示するノード名

	EnemyAttackType							attackType = EnemyAttackType::MeleeCombo;	// 攻撃判定の作り方
	std::array<EnemyAttackStepConfig, 3>	attackSteps{};								// 攻撃モーションのステップごとの設定
	int										attackStepCount = 1;						// 攻撃モーションのステップ数

	float	modelScale				= 1.0f;	// モデルの大きさを変える倍率
	float	moveSpeed				= 1.0f;	// 移動速度
	float	minAttackRange			= 0.0f;	// 攻撃を開始する距離
	float	maxAttackRange			= 1.0f;	// 攻撃を開始する距離
	float	attackForwardOffset		= 1.0f;	// 攻撃判定の中心を、敵の前方へどれだけずらすか
	float	attackHitRadius			= 1.0f;	// 攻撃判定の半径
	int		attackDamage			= 10;	// 攻撃力

	float	projectileSpeed			= 0.0f;	// 発射速度
	float	projectileRadius		= 0.0f;	// 発射物の半径
	float	projectileLifeTime		= 0.0f;	// 発射物の生存時間
	float	projectileTargetHeight	= 0.0f;	// 目標地点の高さ

	float	footOffsetY				= 0.0f;	// 足のオフセットY
	float	bodyRadius				= 0.5f; // 体の半径
	float	bodyHeight				= 1.8f; // 体の高さ
	float	lockOnHeight			= 1.0f; // ロックオンの高さ

	float	walkAnimationCycleTime	= 0.6f; // 歩行アニメーションの1サイクルの時間
	float	deathAnimationTime		= 1.5f; // 死亡アニメーションの時間
	int		maxParryDurability		= 1;	// パリィ耐久値

	std::array<float, 3> attackWarningColor = { 1.0f, 0.5f, 0.1f }; // 攻撃警告の色
};

// 指定した種類に対応する、実行中に変化しない設定を返す
const EnemyConfig& GetEnemyConfig(EnemyType enemyType);