#pragma once

#include"../BaseScene/BaseScene.h"

class TitleScene : public BaseScene
{
public :

	TitleScene()  {}
	~TitleScene() {}

private :

	void Event()		override;
	void Init()			override;
	void DebugUpdate()	override;
};
