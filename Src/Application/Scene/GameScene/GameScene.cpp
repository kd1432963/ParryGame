#include "GameScene.h"

#include "../SceneManager.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/Player/PlayerController.h"
#include "../../GameObject/Stage/ArenaFloor.h"
#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"

void GameScene::Event()
{
	
}

void GameScene::Init()
{
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
