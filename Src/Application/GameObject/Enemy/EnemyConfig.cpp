#include "EnemyConfig.h"

namespace
{
	EnemyConfig CreateGolemConfig()
	{
		EnemyConfig config;

		config.modelFileName		= "Asset/Models/Enemy/Golem/Golem.gltf";
		config.idleAnimationName	= "EnemyGolem_Idle";
		config.chaseAnimationName	= "EnemyGolem_Walk";
		config.attackAnimationName	= "EnemyGolem_Attack";
		config.hitAnimationName		= "EnemyGolem_Hit";
		config.deathAnimationName	= "EnemyGolem_Death";
		config.attackNodeName		= "mixamorig:RightHand";

		config.attackType		= EnemyAttackType::MeleeCombo;
		config.attackStepCount	= 3;
		config.attackSteps[0]	= { 1.50f, 0.8f / 1.5f };
		config.attackSteps[1]	= { 1.30f, 0.8f / 1.5f };
		config.attackSteps[2]	= { 1.30f, 0.8f / 1.5f };

		config.modelScale			= 2.2f;
		config.moveSpeed			= 2.0f;
		config.minAttackRange		= 0.0f;
		config.maxAttackRange		= 2.0f;
		config.attackForwardOffset	= 1.0f;
		config.attackHitRadius		= 1.0f;
		config.attackDamage			= 10;

		config.footOffsetY				= 0.0f;
		config.bodyRadius				= 0.85f;
		config.bodyHeight				= 2.2f;
		config.lockOnHeight				= 1.2f;
		config.walkAnimationCycleTime	= 0.52f;
		config.deathAnimationTime		= 1.50f;
		config.maxParryDurability		= 3;
		config.attackWarningColor		= { 1.0f, 0.55f, 0.10f };

		return config;
	}

	EnemyConfig CreateResonanceCasterConfig()
	{
		EnemyConfig config;

		config.modelFileName			= "Asset/Models/Enemy/ResonanceCaster/ResonanceCaster.gltf";
		config.idleAnimationName		= "ResonanceCaster_Idle";
		config.chaseAnimationName		= "ResonanceCaster_Walk";
		config.attackAnimationName		= "ResonanceCaster_Attack";
		config.hitAnimationName			= "ResonanceCaster_Hit";
		config.deathAnimationName		= "ResonanceCaster_Death";
		config.attackNodeName			= "mixamorig:Spine2";

		// 近づきすぎると後退し、胸部から低速の共鳴弾を1発撃つ
		config.attackType				= EnemyAttackType::ResonanceProjectile;
		config.attackStepCount			= 1;
		config.attackSteps[0]			= { 1.20f, 0.55f };

		config.modelScale				= 1.5f;
		config.moveSpeed				= 3.2f;
		config.minAttackRange			= 4.5f;
		config.maxAttackRange			= 7.5f;
		config.attackForwardOffset		= 0.45f;
		config.attackHitRadius			= 0.75f;
		config.attackDamage				= 10;

		config.projectileSpeed			= 6.0f;
		config.projectileRadius			= 0.38f;
		config.projectileLifeTime		= 3.0f;
		config.projectileTargetHeight	= 0.9f;

		config.footOffsetY				= 0.0f;
		config.bodyRadius				= 0.50f;
		config.bodyHeight				= 1.85f;
		config.lockOnHeight				= 1.10f;
		config.walkAnimationCycleTime	= 0.72f;
		config.deathAnimationTime		= 1.50f;
		config.maxParryDurability		= 2;
		config.attackWarningColor		= { 0.10f, 0.82f, 1.0f };

		return config;
	}

	const EnemyConfig kGolemConfig				= CreateGolemConfig();
	const EnemyConfig kResonanceCasterConfig	= CreateResonanceCasterConfig();
}

const EnemyConfig& GetEnemyConfig(EnemyType enemyType)
{
	switch (enemyType)
	{
	case EnemyType::ResonanceCaster:
		return kResonanceCasterConfig;

	case EnemyType::Golem:
	default:
		return kGolemConfig;
	}
}
