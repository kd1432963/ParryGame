#include "ResonanceProjectile.h"

#include "../Enemy.h"
#include "../../Player/Player.h"
#include "../../../Combat/AttackInfo.h"
#include "../../../Scene/SceneManager.h"
#include "../../../main.h"

namespace
{
	// 弾の仮表示に使用する画像
	const std::string kProjectileTextureFileName = "Asset/Data/Effect/ResonanceProjectile/Texture/ResonanceOrb.png";

	// 画面へ表示する弾の直径
	constexpr float kProjectileVisualDiameter = 0.85f;

	// 弾が周囲を照らす色と範囲
	const Math::Vector3	kProjectileLightColor	= { 0.10f, 0.70f, 1.00f };
	constexpr float		kProjectileLightRadius	= 5.0f;
}

//===========================================================
// コンストラクタ・デストラクタ
//===========================================================
ResonanceProjectile::ResonanceProjectile(
	const Math::Vector3&			startPosition,
	const Math::Vector3&			direction,
	float							speed,
	float							hitRadius,
	float							lifeTime,
	int								damage,
	const std::shared_ptr<Enemy>&	owner
)
	:	m_wpOwner(owner),
		m_position(startPosition),
		m_direction(direction),
		m_speed(speed),
		m_hitRadius(hitRadius),
		m_lifeTime(lifeTime),
		m_damage(damage)
{
	// 速度の大きさへ影響しないよう、方向だけに正規化する
	if (m_direction.LengthSquared() > 0.0f)
	{
		m_direction.Normalize();
	}
}
ResonanceProjectile::~ResonanceProjectile() = default;

//===========================================================
// 初期化
//===========================================================
void ResonanceProjectile::Init()
{
	m_spBillboard = std::make_shared<KdSquarePolygon>(kProjectileTextureFileName);

	m_spBillboard->Set2DObject(false);
	m_spBillboard->SetScale(kProjectileVisualDiameter);
}

//===========================================================
// 更新
//===========================================================
void ResonanceProjectile::Update()
{
	const auto owner = m_wpOwner.lock();

	// 発射したEnemyが消えたら、残っている弾も削除する
	if (!owner || owner->IsDead())
	{
		m_isExpired = true;
		return;
	}

	const float deltaTime = Application::Instance().GetDeltaTime();

	if (deltaTime <= 0.0f) return;

	// 発射時に決めた方向へ直線移動する
	m_position += m_direction * m_speed * deltaTime;

	// 弾の現在位置へポイントライトを毎フレーム登録する
	KdShaderManager::Instance()
		.WorkAmbientController()
		.AddPointLight(
			kProjectileLightColor,
			kProjectileLightRadius,
			m_position
		);

	m_lifeTime = std::max(0.0f, m_lifeTime - deltaTime);

	m_elapsedTime += deltaTime;

	// Player へ命中、または寿命切れなら削除する
	if (TryHitPlayer() || m_lifeTime <= 0.0f)
	{
		m_isExpired = true;
	}
}

//===========================================================
// Playerとの命中判定
//===========================================================
bool ResonanceProjectile::TryHitPlayer()
{
	KdCollider::SphereInfo projectileSphere(
		KdCollider::TypeDamage,
		m_position,
		m_hitRadius
	);

	for (const auto& object :
		SceneManager::Instance().GetObjList())
	{
		const auto player = std::dynamic_pointer_cast<Player>(object);

		if (!player || player->IsDead()) continue;

		std::list<KdCollider::CollisionResult> results;

		if (!player->Intersects(projectileSphere,&results))continue;

		AttackInfo attackInfo;
		attackInfo.damage = m_damage;
		attackInfo.hitPos = m_position;
		attackInfo.attacker = m_wpOwner;

		// 複数の接触結果がある場合は、最も深く当たった位置を採用する
		if (!results.empty())
		{
			const auto bestResult = std::max_element(
				results.begin(),
				results.end(),
				[](const auto& left, const auto& right)
				{
					return
						left.m_overlapDistance <
						right.m_overlapDistance;
				}
			);

			attackInfo.hitPos =
				bestResult->m_hitPos;
		}

		player->OnHit(attackInfo);

		return true;
	}

	return false;
}

//===========================================================
// エフェクト描画
//===========================================================
void ResonanceProjectile::DrawEffect()
{
	if (!m_spBillboard) return;

	// 時間に応じて少し膨張・収縮させる
	const float pulse = 0.5f + 0.5f *
		sinf(
			m_elapsedTime *
			DirectX::XM_2PI *
			5.0f
		);

	const float scale = kProjectileVisualDiameter * (0.92f + pulse * 0.16f);

	m_spBillboard->SetScale(scale);
	m_spBillboard->SetColor(
		Math::Color(
			0.40f,
			0.90f,
			1.00f,
			0.92f
		)
	);

	// カメラ行列を利用して、板ポリゴンを常にカメラへ向ける
	Math::Matrix world =
		KdShaderManager::Instance()
		.GetCameraCB()
		.mView
		.Invert();

	world.Translation(m_position);

	auto& shaderManager = KdShaderManager::Instance();

	shaderManager.ChangeDepthStencilState(
		KdDepthStencilState::ZWriteDisable
	);
	shaderManager.ChangeBlendState(KdBlendState::Add);

	shaderManager.m_StandardShader.DrawPolygon(
		*m_spBillboard,
		world,
		kWhiteColor,
		Math::Vector3::Zero
	);

	shaderManager.UndoBlendState();
	shaderManager.UndoDepthStencilState();
}