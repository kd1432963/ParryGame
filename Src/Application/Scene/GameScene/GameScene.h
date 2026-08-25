#pragma once

#include"../BaseScene/BaseScene.h"

class CameraBase;
class LockOnTargetManager;
class Player;
class ScreenFlash;
class CharacterSeparation;
class BattleFlow;
class PlayerController;

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

	//===========================================================
	// 所有する管理クラス
	//===========================================================
	std::shared_ptr<LockOnTargetManager>	m_spLockOnTargetManager	= nullptr;
	std::shared_ptr<CharacterSeparation>	m_spCharacterSeparation	= nullptr;
	std::shared_ptr<BattleFlow>				m_spBattleFlow			= nullptr;
	std::shared_ptr<PlayerController>		m_spPlayerController	= nullptr;

	//===========================================================
	// 状態値
	//===========================================================
	bool m_wasEnterKeyDown = false;	// 1フレーム前のEnter入力
};
