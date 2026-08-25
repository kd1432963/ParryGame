#pragma once

#include"../BaseScene/BaseScene.h"

class Player;
class BaseCamera;
class ScreenFlash;;

class GameScene : public BaseScene
{
public :

	GameScene()  {}
	~GameScene() {}

private:

	void Event()		override;
	void Init()			override;
	void DebugUpdate()	override;

	void SetupParrySuccessEffect(
		const std::shared_ptr<Player>&		player,
		const std::shared_ptr<CameraBase>&	camera,
		const std::shared_ptr<ScreenFlash>& screenFlash
	);
	void SetupDamageEffect(
		const std::shared_ptr<Player>& player,
		const std::shared_ptr<CameraBase>& camera,
		const std::shared_ptr<ScreenFlash>& screenFlash
	);
};
