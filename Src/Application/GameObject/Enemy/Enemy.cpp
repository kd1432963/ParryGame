#include "Enemy.h"

#include "../../main.h"
#include "../../Animation/AnimationPlayer.h"
#include "../Physics/GroundPhysics.h"
#include "../Enemy/EnemyState.h"
#include "../../StateMachine/StateMachine.h"
#include "../Player/Player.h"
#include "../Enemy/EnemyConfig.h"
#include "../../Combat/AttackHitBox.h"
#include "../../Combat/AttackInfo.h"
#include "../../Scene/SceneManager.h"
#include "ResonanceProjectile/ResonanceProjectile.h"

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
Enemy::Enemy(EnemyType type)
	: m_upAnimationPlayer(std::make_unique<AnimationPlayer>()),
		m_upGroundPhysics(std::make_unique<GroundPhysics>()),
		m_upStateMachine(std::make_unique<StateMachine<EnemyStateId>>()),
		m_upConfig(std::make_unique<EnemyConfig>(
		GetEnemyConfig(type)
	)),
		m_parryDurability(m_upConfig->maxParryDurability)
{
}
Enemy::~Enemy() = default;

//===========================================================
// 初期化関数
//===========================================================
void Enemy::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();

		m_spModel->SetModelData(
			KdAssets::Instance().m_modeldatas.GetData(
				m_upConfig->modelFileName
			)
		);
	}

#ifdef _DEBUG
	if (!m_pDebugWire)
	{
		m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	}
#endif

	// 初期座標の設定
	SetPos({ 0.0f, 0.0f, 5.0f });
	UpdateWorldMatrix(GetPos());

	// ステートマシンの状態登録
	SetupStateMachine();
}

//===========================================================
// 更新関数
//===========================================================
void Enemy::Update()
{
	const float	deltaTime	= Application::Instance().GetDeltaTime();
	const auto	player		= m_wpPlayer.lock();

	// 現在状態の Update だけを実行する
	m_upStateMachine->Update(deltaTime);

	if (m_spModel)
	{
		m_upAnimationPlayer->Update(*m_spModel, deltaTime);
	}

	// 水平移動が終わったあと、最後に高さを確定する
	m_upGroundPhysics->Update(
		*this,
		deltaTime,
		m_upConfig->footOffsetY
	);
}

//===========================================================
// 描画関数
//===========================================================
void Enemy::DrawLit()
{
	if (!m_spModel)return;
	auto& standardShader = KdShaderManager::Instance().m_StandardShader;

	// Dead中だけノイズ境界付きのディゾルブを有効にする
	if (m_dissolveProgress > 0.0f)
	{
		const float			dissolveEdgeRange = kDissolveEdgeRange;
		const Math::Vector3	dissolveEdgeColor = { 1.0f, 0.25f, 0.02f };

		standardShader.SetDissolve(
			m_dissolveProgress,
			&dissolveEdgeRange,
			&dissolveEdgeColor
		);
	}

	//const Math::Color color = GetDrawColor();

	standardShader.DrawModel(
		*m_spModel,
		m_mWorld
		//color
	);
}

//===========================================================
// 攻撃を受けたときの処理
//===========================================================
ParryResult Enemy::OnParried()
{
	--m_parryDurability;

	// 撃破済みなら死亡状態に遷移する
	if (m_parryDurability <= 0)
	{
		m_upStateMachine->ChangeState(EnemyStateId::Dead);

		return ParryResult::Defeated;
	}

	// 近接型だけは連撃の途中で止めず、次のパリィ機会を作る
	if (m_upConfig->attackType == EnemyAttackType::MeleeCombo &&
		m_comboAttackIndex < m_upConfig->attackStepCount)
	{
		return ParryResult::ContinueCombo;
	}

	// 撃破されていない場合は怯み状態に遷移する
	m_upStateMachine->ChangeState(EnemyStateId::Stun);

	return ParryResult::Stagger;
}

//===========================================================
// カメラが狙う敵の胴体位置を取得する
//===========================================================
Math::Vector3 Enemy::GetLockOnPosition() const
{
	Math::Vector3 lockOnPosition = GetPos();

	if (m_upConfig)
	{
		lockOnPosition.y += m_upConfig->lockOnHeight;
	}

	return lockOnPosition;
}

//===========================================================
// 胸ボーンの現在位置をロックオンマーカーへ渡す
//===========================================================
Math::Vector3 Enemy::GetLockOnMarkerPosition() const
{
	if (m_spModel &&
		m_upConfig &&
		!m_upConfig->lockOnNodeName.empty())
	{
		// アニメーション後の全ボーン座標を必要な場合だけ計算する
		if (m_spModel->NeedCalcNodeMatrices())
		{
			m_spModel->CalcNodeMatrices();
		}

		const auto* lockOnNode = m_spModel->FindNode(m_upConfig->lockOnNodeName);

		if (lockOnNode)
		{
			const Math::Matrix lockOnWorld = lockOnNode->m_worldTransform * m_mWorld;

			return lockOnWorld.Translation();
		}
	}

	// ボーンが存在しないモデルでは従来の固定位置を使う
	return GetLockOnPosition();
}

//===========================================================
// 現在のパリィ耐久値を取得する
//===========================================================
int Enemy::GetCurrentParryDurability() const
{
	return m_parryDurability;
}

//===========================================================
// パリィ耐久値の最大値を取得する
//===========================================================
int Enemy::GetMaxParryDurability() const
{
	if (!m_upConfig) return 0;

	return m_upConfig->maxParryDurability;
}

//===========================================================
// プレイヤーとの距離に応じて移動する関数
//===========================================================
float Enemy::UpdateMove(
	const std::shared_ptr<Player>& player,
	float							distance,
	float							deltaTime
)
{
	Math::Vector3 enemyPos = GetPos();

	// 設定された得意距離内なら、その場で向きだけ反映する
	if (distance >= m_upConfig->minAttackRange &&
		distance <= m_upConfig->maxAttackRange)
	{
		UpdateWorldMatrix(enemyPos);
		return distance;
	}

	Math::Vector3 moveDirection = player->GetPos() - enemyPos;
	moveDirection.y = 0.0f;

	if (moveDirection.LengthSquared() <= 0.0f)
	{
		UpdateWorldMatrix(enemyPos);
		return 0.0f;
	}

	moveDirection.Normalize();

	float moveDistance = 0.0f;

	if (distance > m_upConfig->maxAttackRange)
	{
		// 遠い場合は、得意距離の外側まで近づく
		moveDistance = std::min(
			m_upConfig->moveSpeed * deltaTime,
			distance - m_upConfig->maxAttackRange
		);
		enemyPos += moveDirection * moveDistance;
		distance -= moveDistance;
	}
	else if (distance < m_upConfig->minAttackRange)
	{
		// 射手だけは近づかれた分を後退して間合いを作る
		moveDistance = std::min(
			m_upConfig->moveSpeed * deltaTime,
			m_upConfig->minAttackRange - distance
		);
		enemyPos -= moveDirection * moveDistance;
		distance += moveDistance;
	}

	UpdateWorldMatrix(enemyPos);

	return distance;
}

//===========================================================
// プレイヤーの方向を向く関数
//===========================================================
float Enemy::UpdateDirection(const std::shared_ptr<Player>& player)
{
	// プレイヤーへの方向を計算する
	const Math::Vector3	enemyPos = GetPos();
	const Math::Vector3	playerPos = player->GetPos();
	Math::Vector3		toPlayer = playerPos - enemyPos;

	// 高さの差を追跡距離へ含めない
	toPlayer.y = 0.0f;

	const float distance = toPlayer.Length();

	if (distance <= 0.0f) return 0.0f;

	toPlayer.Normalize();

	// Enemy モデルの正面をプレイヤーへ向ける
	m_yaw = atan2f(-toPlayer.x, -toPlayer.z);
	UpdateWorldMatrix(enemyPos);

	return distance;
}

//===========================================================
// ワールド行列更新関数
//===========================================================
void Enemy::UpdateWorldMatrix(
	const Math::Vector3&	pos,
	float					effectScaleXZ,
	float					effectScaleY
)
{
	const Math::Matrix scale = Math::Matrix::CreateScale(
		m_upConfig->modelScale * effectScaleXZ,
		m_upConfig->modelScale * effectScaleY,
		m_upConfig->modelScale * effectScaleXZ
	);

	const Math::Matrix	rotation	= Math::Matrix::CreateRotationY(m_yaw);
	const Math::Matrix	translation = Math::Matrix::CreateTranslation(pos);

	m_mWorld = scale * rotation * translation;
}

//===========================================================
// コンボ攻撃開始関数
//===========================================================
void Enemy::StartComboAttack()
{
	// 攻撃ステップの設定を取得
	const EnemyAttackStepConfig&	stepConfig	= m_upConfig->attackSteps[m_comboAttackIndex];
	const float						duration	= stepConfig.durationSeconds;

	// この攻撃で実際に攻撃判定が発生するまでの時間を保存
	m_currentAttackHitDelay = duration * stepConfig.hitTimingRate;

	// 攻撃アニメーションを再生する
	m_upAnimationPlayer->Play(
		*m_spModel,
		m_upConfig->attackAnimationName,
		false,
		duration,
		kAttackBlendTime,
		true
	);

	// 攻撃中フラグを立てる
	m_isAttacking		= true;
	m_attackTimer		= duration;
	m_attackHitTimer	= 0.0f;
}

//===========================================================
// 攻撃判定生成関数
//===========================================================
void Enemy::CreateAttack()
{
	switch (m_upConfig->attackType)
	{
	case EnemyAttackType::ResonanceProjectile:
		CreateResonanceProjectile();
		break;

	case EnemyAttackType::MeleeCombo:
	default:
		CreateAttackHitBox();
		break;
	}
}

//===========================================================
// 攻撃判定生成関数（近接攻撃）
//===========================================================
void Enemy::CreateAttackHitBox()
{
	const Math::Vector3 attackPos = GetAttackPosition();

	AttackInfo attackInfo;
	attackInfo.damage = m_upConfig->attackDamage;
	attackInfo.attacker = shared_from_this();

	const auto hitBox = std::make_shared<AttackHitBox>(
		attackPos,
		m_upConfig->attackHitRadius,
		attackInfo
	);

	SceneManager::Instance().AddObject(hitBox);
}

//===========================================================
// 攻撃判定生成関数（共鳴弾）
//===========================================================
void Enemy::CreateResonanceProjectile()
{
	const auto player = m_wpPlayer.lock();

	if (!player || player->IsDead()) return;

	const Math::Vector3 attackPosition = GetAttackPosition();
	Math::Vector3 targetPosition = player->GetPos();
	targetPosition.y += m_upConfig->projectileTargetHeight;

	Math::Vector3 direction = targetPosition - attackPosition;

	if (direction.LengthSquared() <= 0.0f) return;

	direction.Normalize();

	const auto projectile = std::make_shared<ResonanceProjectile>(
		attackPosition,
		direction,
		m_upConfig->projectileSpeed,
		m_upConfig->projectileRadius,
		m_upConfig->projectileLifeTime,
		m_upConfig->attackDamage,
		std::static_pointer_cast<Enemy>(shared_from_this())
	);
	projectile->Init();

	SceneManager::Instance().AddObject(projectile);
}

//===========================================================
// 攻撃判定の中心位置を取得する関数
//===========================================================
Math::Vector3 Enemy::GetAttackPosition()
{
	if (m_spModel)
	{
		// Animator が更新したローカル行列から、現在の全身姿勢を確定する
		m_spModel->CalcNodeMatrices();

		const auto* attackNode = m_spModel->FindNode(m_upConfig->attackNodeName);

		if (attackNode)
		{
			const Math::Matrix attackWorld = attackNode->m_worldTransform * m_mWorld;
			return attackWorld.Translation();
		}
	}

	// 対象モデルに右手ノードがない場合だけ、前方位置を使う
	Math::Vector3 forward = Math::Vector3::TransformNormal(
		Math::Vector3::Forward,
		m_mWorld
	);

	if (forward.LengthSquared() > 0.0f)
	{
		forward.Normalize();
	}

	return GetPos() + forward * m_upConfig->attackForwardOffset;
}