#include "TitleScene.h"
#include "../SceneManager.h"

void TitleScene::Event()
{
	
}

void TitleScene::Init()
{

}

void TitleScene::DebugUpdate()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}
}
