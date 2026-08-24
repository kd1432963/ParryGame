#include "BaseScene.h"

//============================================================
// 更新前処理
//============================================================
void BaseScene::PreUpdate()
{
	// 無効になったオブジェクトをリストから削除する
	auto it = m_objList.begin();

	while (it != m_objList.end())
	{
		if ((*it)->IsExpired())
		{
			it = m_objList.erase(it);
		}
		else
		{
			++it;
		}
	}

	// 生存している全オブジェクトへ更新前処理を渡す
	for (auto& obj : m_objList)
	{
		obj->PreUpdate();
	}
}

//============================================================
// 更新処理
//============================================================
void BaseScene::Update()
{
	// 派生シーン固有の処理を実行する
	Event();

#ifdef _DEBUG
	// 開発中だけ必要なキー操作や調整処理
	DebugUpdate();
#endif

	// 全オブジェクトを更新する
	for (auto& obj : m_objList)
	{
		obj->Update();
	}
}

//============================================================
// 更新後処理
//============================================================
void BaseScene::PostUpdate()
{
	for (auto& obj : m_objList)
	{
		obj->PostUpdate();
	}
}

//============================================================
// 描画前処理
//============================================================
void BaseScene::PreDraw()
{
	for (auto& obj : m_objList)
	{
		obj->PreDraw();
	}
}

//============================================================
// 3D描画
//============================================================
void BaseScene::Draw()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光を遮るオブジェクト(影を生み出す要因となるオブジェクト)をBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginGenerateDepthMapFromLight();
	{
		for (auto& obj : m_objList)
		{
			obj->GenerateDepthMapFromLight();
		}
	}
	KdShaderManager::Instance().m_StandardShader.EndGenerateDepthMapFromLight();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のないオブジェクト(背景など)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawUnLit();
		}
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のあるオブジェクト(光源の影響を受けるオブジェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginLit();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawLit();
		}
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のないオブジェクト(エフェクトなど)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawEffect();
		}
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光源オブジェクト(自ら光るオブジェクトやエフェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_postProcessShader.BeginBright();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawBright();
		}
	}
	KdShaderManager::Instance().m_postProcessShader.EndBright();
}

//============================================================
// 2D描画
//============================================================
void BaseScene::DrawSprite()
{
	KdShaderManager::Instance().m_spriteShader.Begin();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawSprite();
		}
	}
	KdShaderManager::Instance().m_spriteShader.End();
}

//============================================================
// デバッグ描画
//============================================================
void BaseScene::DrawDebug()
{
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawDebug();
		}
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();
}

//============================================================
// 開発用更新処理
//============================================================
void BaseScene::DebugUpdate()
{
	// 各シーンで必要な内容を実装(オーバーライド)する
}

//============================================================
// 派生シーン固有の処理
//============================================================
void BaseScene::Event()
{
	// 各シーンで必要な内容を実装(オーバーライド)する
}
