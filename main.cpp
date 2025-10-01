#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"
#include <Windows.h>

using namespace KamataEngine;

TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
};

Scene scene = Scene::kUnknown;

void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:

		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			delete gameScene;
			gameScene = nullptr;
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene() {

	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2C_01_アンドウ_カナデ_AL3");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	scene = Scene::kTitle;
	titleScene = new TitleScene;
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		// シーン切り替え
		ChangeScene();
		// シーン更新
		UpdateScene();

		// ImGui受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		// シーンの描画
		DrawScene();

		// 軸表示の描画
		AxisIndicator::GetInstance()->Draw();

		// プリミティブ描画のリセット
		PrimitiveDrawer::GetInstance()->Reset();

		// ImGui描画
		imguiManager->Draw();
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	delete titleScene;
	delete gameScene;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
