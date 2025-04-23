#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize();

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

#pragma region メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {

			break;
		}

		gameScene->Update();

#pragma region 描画
		dxCommon->PreDraw();

		gameScene->Draw();

		dxCommon->PostDraw();
#pragma endregion
	}

	delete gameScene;
	gameScene = nullptr;

	KamataEngine::Finalize();
#pragma endregion

	return 0;
}
