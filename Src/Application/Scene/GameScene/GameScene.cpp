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
#include "../../GameObject/Camera/CameraManager.h"
#include "../../GameObject/Camera/LockOnController/LockOnController.h"
#include "../../GameObject/UI/LockOnMarker/LockOnMarker.h"
#include "../../Combat/LockOn/LockOnTargetManager.h"
#include "../../GameObject/Physics/CharacterSeparation.h"
#include "../../GameFlow/BattleFlow.h"
#include "../../GameObject/UI/BattleResultUI/BattleResultUI.h"

void GameScene::Event()
{
	if (!m_spBattleFlow || !m_spPlayerController) return;

	// 押し続けではなく、Enterを押した瞬間だけを取得する
	const bool	isEnterKeyDown	= (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;
	const bool	isEnterPressed	= isEnterKeyDown && !m_wasEnterKeyDown;

	m_wasEnterKeyDown = isEnterKeyDown;

	// 戦闘終了後はプレイヤーへの操作入力を停止する
	const bool isPlaying = m_spBattleFlow->GetState() == BattleState::Playing;

	m_spPlayerController->SetInputEnabled(isPlaying);

	// 終了画面でEnterが押されたら、現在のGameSceneを最初から作り直す
	if (isPlaying || !isEnterPressed) return;

	SceneManager::Instance().RequestReload();
}

void GameScene::Init()
{
	// モデルの色が暗く潰れないように環境光を設定する
	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(
		Math::Vector4(0.85f, 0.85f, 0.90f, 1.0f)
	);

	//==========================================================
	// ロックオン対象管理クラス生成
	//==========================================================
	m_spLockOnTargetManager = std::make_shared<LockOnTargetManager>();

	//==========================================================
	// キャラクター押し戻し管理クラス生成
	//==========================================================
	m_spCharacterSeparation = std::make_shared<CharacterSeparation>();

	// カメラより先に PostUpdate して、補正後の座標を追従させる
	AddObject(m_spCharacterSeparation);

	//==========================================================
	// ステージ生成
	//==========================================================
	auto arenaFloor = std::make_shared<ArenaFloor>();
	arenaFloor->Init();
	AddObject(arenaFloor);

	//==========================================================
	// カメラ生成
	//==========================================================
	auto cameraManager = std::make_shared<CameraManager>();
	auto camera = std::make_shared<TPSCamera>();
	camera->SetLockOnTargetManager(m_spLockOnTargetManager);
	camera->Init();
	cameraManager->RegisterCamera(
		CameraManager::CameraType::ThirdPerson
		,camera);
	AddObject(cameraManager);

	//==========================================================
	// プレイヤー生成
	//==========================================================
	auto player = std::make_shared<Player>();
	player->Init();
	m_spCharacterSeparation->RegisterBody(
		player,
		player->GetBodyRadius(),
		player->GetBodyHeight()
	);
	AddObject(player);

	//==========================================================
	// バトル進行管理クラス生成
	//==========================================================
	m_spBattleFlow = std::make_shared<BattleFlow>();
	m_spBattleFlow->SetPlayer(player);

	//==========================================================
	// 近接型の敵生成
	//==========================================================
	auto golem = std::make_shared<Enemy>(EnemyType::Golem);
	golem->Init();
	golem->SetPos({ -2.5f, 0.0f, 6.0f });
	golem->SetPlayer(player);
	m_spBattleFlow->RegisterEnemy(golem);

	m_spCharacterSeparation->RegisterBody(
		golem,
		golem->GetBodyRadius(),
		golem->GetBodyHeight()
	);

	AddObject(golem);

	AddObject(m_spBattleFlow);

	//==========================================================
	// 遠距離型の敵生成
	//==========================================================
	auto resonanceCaster = std::make_shared<Enemy>(
		EnemyType::ResonanceCaster
	);

	resonanceCaster->Init();
	resonanceCaster->SetPos({ 3.5f, 0.0f, 8.0f });
	resonanceCaster->SetPlayer(player);
	m_spBattleFlow->RegisterEnemy(resonanceCaster);

	m_spCharacterSeparation->RegisterBody(
		resonanceCaster,
		resonanceCaster->GetBodyRadius(),
		resonanceCaster->GetBodyHeight()
	);

	AddObject(resonanceCaster);


	//==========================================================
	// プレイヤーコントローラー生成
	//==========================================================
	m_spPlayerController = std::make_shared<PlayerController>();
	m_spPlayerController->SetPlayer(player);
	m_spPlayerController->SetCamera(camera);
	m_spPlayerController->SetLockOnTargetManager(m_spLockOnTargetManager);
	AddObject(m_spPlayerController);

	//==========================================================
	// ロックオンコントローラー生成
	//==========================================================
	auto lockOnController = std::make_shared<LockOnController>();
	lockOnController->SetCameraManager(cameraManager);
	lockOnController->SetLockOnTargetManager(m_spLockOnTargetManager);
	lockOnController->SetPlayer(player);
	AddObject(lockOnController);

	//==========================================================
	// ロックオンマーカー生成
	//==========================================================
	auto lockOnMarker = std::make_shared<LockOnMarker>();
	lockOnMarker->SetCameraManager(cameraManager);
	AddObject(lockOnMarker);

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

	//==========================================================
	// バトル結果UI生成
	//==========================================================
	auto battleResultUI = std::make_shared<BattleResultUI>();
	battleResultUI->SetBattleFlow(m_spBattleFlow);
	battleResultUI->Init();
	AddObject(battleResultUI);
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

			// 敵を倒しているかどうか
			const bool didParryDefeatEnemy = result == ParryResult::Defeated;

			//==========================================================
			// 敵が死んでいなければ
			//==========================================================
			if (!didParryDefeatEnemy)
			{
				// パリィした敵をロックオン対象にする
				const auto tpsCamera	= std::dynamic_pointer_cast<TPSCamera>(camera);
				const auto lockOnTarget = std::dynamic_pointer_cast<ILockOnTarget>(parriedObj);

				if (tpsCamera && lockOnTarget)
				{
					tpsCamera->SetLockOnTarget(lockOnTarget);
				}
			}

			//==========================================================
			// カメラ揺れの設定
			//==========================================================
			CameraShakeSettings shakeSettings{};

			shakeSettings.durationSeconds		= 0.22f;
			shakeSettings.samplesPerSecond		= 40.0f;
			shakeSettings.maxPositionOffset		= Math::Vector3(0.1f, 0.1f, 0.0f);
			shakeSettings.maxRotationDegrees	= Math::Vector3(0.5f, 0.5f, 0.5f);

			// 敵を倒したパリィは強く揺らす
			if (didParryDefeatEnemy)
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
			if (didParryDefeatEnemy)
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
			float hitStopDuration	= 0.22f;
			float hitStopTimeScale	= 0.2f;

			// 敵を倒したズームは長くヒットストップする
			if (didParryDefeatEnemy)
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

			flashSettings.color			= { 0.35f, 0.85f, 1.0f };
			flashSettings.peakOpacity	= 0.12f;
			flashSettings.fadeDuration	= 0.10f;

			if (didParryDefeatEnemy)
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
