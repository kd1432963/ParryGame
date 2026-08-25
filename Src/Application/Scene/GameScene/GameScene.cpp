#include "GameScene.h"

#include "../SceneManager.h"
#include "../../main.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/Player/PlayerController.h"
#include "../../GameObject/Stage/ArenaFloor.h"
#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Enemy/Enemy.h"
#include "../../GameObject/Enemy/EnemyConfig.h"
#include "../../GameObject/Camera/CameraShake/CameraShake.h"
#include "../../GameObject/Camera/CameraZoom/CameraZoom.h"
#include "../../GameObject/Camera/CameraBase.h"
#include "../../Combat/IParryable.h"
#include "../../Time/HitStop.h"
#include "../../GameObject/Effect/ParryEffect/ParryEffect.h"
#include "../../GameObject/Effect/ScreenFlash/ScreenFlash.h"

void GameScene::Event()
{
	
}

void GameScene::Init()
{
	// モデルの色が暗く潰れないように環境光を設定する
	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(Math::Vector4(0.85f, 0.85f, 0.90f, 1.0f));

	//==========================================================
	// ステージ生成
	//==========================================================
	auto arenaFloor = std::make_shared<ArenaFloor>();
	arenaFloor->Init();
	AddObject(arenaFloor);

	//==========================================================
	// カメラ生成
	//==========================================================
	auto camera = std::make_shared<TPSCamera>();
	camera->Init();
	camera->StartShake(0.08f, 0.20f);
	AddObject(camera);

	//==========================================================
	// プレイヤー生成
	//==========================================================
	auto player = std::make_shared<Player>();
	player->Init();
	AddObject(player);

	//==========================================================
	// 敵生成
	//==========================================================
	auto enemy = std::make_shared<Enemy>(EnemyType::ResonanceCaster);
	enemy->Init();
	enemy->SetPlayer(player);
	AddObject(enemy);

	enemy = std::make_shared<Enemy>(EnemyType::Golem);
	enemy->Init();
	enemy->SetPlayer(player);
	AddObject(enemy);

	//==========================================================
	// プレイヤーコントローラー生成
	//==========================================================
	auto playerController = std::make_shared<PlayerController>();
	playerController->SetPlayer(player);
	playerController->SetCamera(camera);
	AddObject(playerController);

	//==========================================================
	// カメラのターゲットをプレイヤーに設定
	//==========================================================
	camera->SetTarget(player);

	//==========================================================
	// エフェクト生成
	//==========================================================
	auto screenFlash = std::make_shared<ScreenFlash>();
	AddObject(screenFlash);

	//==========================================================
	// パリィ成功時のエフェクト
	//==========================================================
	SetupParrySuccessEffect(player, camera, screenFlash);

	//==========================================================
	// ダメージ時のエフェクト
	//==========================================================
	SetupDamageEffect(player, camera, screenFlash);
}

//==========================================================
// デバッグ用更新処理
//==========================================================
void GameScene::DebugUpdate()
{
	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}

//==========================================================
// パリィ成功時のエフェクト
//==========================================================
void GameScene::SetupParrySuccessEffect(
	const std::shared_ptr<Player>&		player,
	const std::shared_ptr<CameraBase>&	camera,
	const std::shared_ptr<ScreenFlash>& screenFlash
)
{
	const std::weak_ptr<CameraBase>		wpCamera		= camera;
	const std::weak_ptr<ScreenFlash>	wpScreenFlash	= screenFlash;

	player->SetParrySuccessCallback(
		[wpCamera, wpScreenFlash](
			const Math::Vector3&					hitPos,
			ParryResult								result,
			const std::shared_ptr<KdGameObject>&	parriedObj
			)
		{
			const auto camera = wpCamera.lock();
			if (!camera) return;

			const auto screenFlash = wpScreenFlash.lock();
			if (!screenFlash) return;

			//==========================================================
			// カメラ揺れの設定
			//==========================================================
			CameraShakeSettings shakeSettings{};

			shakeSettings.durationSeconds		= 0.22f;
			shakeSettings.samplesPerSecond		= 40.0f;
			shakeSettings.maxPositionOffset		= Math::Vector3(0.1f, 0.1f, 0.0f);
			shakeSettings.maxRotationDegrees	= Math::Vector3(0.5f, 0.5f, 0.5f);

			// 敵を倒したパリィは強く揺らす
			if (result == ParryResult::Defeated)
			{
				shakeSettings.durationSeconds		= 0.45f;
				shakeSettings.maxPositionOffset		= Math::Vector3(0.48f, 0.34f, 0.30f);
				shakeSettings.maxRotationDegrees	= Math::Vector3(1.20f, 0.85f, 1.60f);
			}

			camera->StartShake(shakeSettings);

			//==========================================================
			// カメラズームの設定
			//==========================================================
			CameraZoomSettings zoomSettings{};

			zoomSettings.targetFieldOfView	= 50.0f;
			zoomSettings.zoomInDuration		= 0.045f;
			zoomSettings.holdDuration		= 0.06f;
			zoomSettings.zoomOutDuration	= 0.28f;

			// 敵を倒したズームは強くズームする
			if (result == ParryResult::Defeated)
			{
				zoomSettings.targetFieldOfView	= 45.0f;
				zoomSettings.zoomInDuration		= 0.05f;
				zoomSettings.holdDuration		= 0.06f;
				zoomSettings.zoomOutDuration	= 0.28f;
			}

			camera->StartZoom(zoomSettings);

			//==========================================================
			// ヒットストップの設定
			//==========================================================
			float hitStopDuration = 0.22f;
			float hitStopTimeScale = 0.2f;

			// 敵を倒したズームは長くヒットストップする
			if (result == ParryResult::Defeated)
			{
				hitStopDuration		= 0.32f;
				hitStopTimeScale	= 0.2f;
			}

			Application::Instance().StartHitStop(hitStopDuration, hitStopTimeScale);

			//==========================================================
			// パリィエフェクトの生成
			//==========================================================
			auto parryEffect = std::make_shared<ParryEffect>();
			parryEffect->Play(hitPos);

			//==========================================================
			// スクリーンフラッシュを始める
			//==========================================================
			ScreenFlashSettings flashSettings{};

			flashSettings.color = { 0.35f, 0.85f, 1.0f };
			flashSettings.peakOpacity = 0.12f;
			flashSettings.fadeDuration = 0.10f;

			if (result == ParryResult::Defeated)
			{
				flashSettings.color			= { 1.0f, 0.75f, 0.20f };
				flashSettings.peakOpacity	= 0.20f;
				flashSettings.fadeDuration	= 0.18f;
			}

			screenFlash->Start(flashSettings);
		}
	);
}

//==========================================================
// ダメージ時のエフェクト
//==========================================================
void GameScene::SetupDamageEffect(
	const std::shared_ptr<Player>&		player, 
	const std::shared_ptr<CameraBase>&	camera,
	const std::shared_ptr<ScreenFlash>& screenFlash)
{
	const std::weak_ptr<CameraBase>		wpCamera		= camera;
	const std::weak_ptr<ScreenFlash>	wpScreenFlash	= screenFlash;

	player->SetDamageHitCallback(
		[wpCamera, wpScreenFlash](const Math::Vector3& hitPos)
		{
			const auto camera = wpCamera.lock();
			if (!camera) return;

			const auto screenFlash = wpScreenFlash.lock();
			if (!screenFlash) return;

			//==========================================================
			// カメラ揺れの設定
			//==========================================================
			CameraShakeSettings shakeSettings{};
			shakeSettings.durationSeconds		= 0.22f;
			shakeSettings.samplesPerSecond		= 40.0f;
			shakeSettings.maxPositionOffset		= Math::Vector3(0.1f, 0.1f, 0.0f);
			shakeSettings.maxRotationDegrees	= Math::Vector3(0.5f, 0.5f, 0.5f);
			camera->StartShake(shakeSettings);

			//==========================================================
			// カメラズームの設定
			//==========================================================
			CameraZoomSettings zoomSettings{};
			zoomSettings.targetFieldOfView	= 50.0f;
			zoomSettings.zoomInDuration		= 0.045f;
			zoomSettings.holdDuration		= 0.06f;
			zoomSettings.zoomOutDuration	= 0.28f;
			camera->StartZoom(zoomSettings);
			
			//==========================================================
			// ヒットストップの設定
			//==========================================================
			Application::Instance().StartHitStop(0.22f, 0.2f);
			
			//==========================================================
			// スクリーンフラッシュを始める
			//==========================================================
			ScreenFlashSettings flashSettings{};
			flashSettings.color			= { 1.0f, 0.15f, 0.10f };
			flashSettings.peakOpacity	= 0.20f;
			flashSettings.fadeDuration	= 0.14f;

			screenFlash->Start(flashSettings);
		}
	);
}
