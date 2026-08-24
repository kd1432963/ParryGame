#pragma once

class Stage : public KdGameObject
{
public:

	Stage()						= default;
	~Stage()		override	= default;

	void Init()		override;
	void DrawLit()	override;

private:

	std::shared_ptr<KdModelData>m_spModel = nullptr;
};