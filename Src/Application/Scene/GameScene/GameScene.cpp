#include "GameScene.h"

#include "../SceneManager.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/Player/PlayerController.h"
#include "../../GameObject/Stage/ArenaFloor.h"
#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../GameObject/Enemy/Enemy.h"
#include "../../GameObject/Enemy/EnemyConfig.h"

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
