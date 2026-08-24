#pragma once

class CameraBase;

//===========================================================
// シーンの基底クラス
//===========================================================
class BaseScene
{
public :

	//===========================================================
	// コンストラクタ・デストラクタ
	//===========================================================
	BaseScene()				{}
	virtual ~BaseScene()	{}

	//===========================================================
	// 基本ライフサイクル
	//===========================================================
	virtual void Init() = 0;

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	// オブジェクトリストを取得
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList()
	{
		return m_objList;
	}
	
	// オブジェクトリストに追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_objList.push_back(_obj);
	}

protected :

	// 継承先シーンで必要ならオーバーライドする
	virtual void Event();

	// Debug ビルド時だけ呼ばれる開発用更新処理
	virtual void DebugUpdate();

	// カメラオブジェクト
	std::shared_ptr<CameraBase>					m_Camera = nullptr;

	// 全オブジェクトのアドレスをリストで管理
	std::list<std::shared_ptr<KdGameObject>>	m_objList;
};
