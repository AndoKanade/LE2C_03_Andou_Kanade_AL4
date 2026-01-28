#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"
#include "RuleScene.h"
#include "endScene.h"
#include <Windows.h>
#include "ParticleManager.h"

using namespace KamataEngine;

TitleScene* titleScene = nullptr;
RuleScene* ruleScene = nullptr;
GameScene* gameScene = nullptr;
EndScene* endScene = nullptr;


enum class Scene {
	kUnknown = 0,
	kTitle,
	kRule,
	kGame,
	kEnd,
};

Scene scene = Scene::kUnknown;

void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kRule;
			delete titleScene;
			titleScene = nullptr;
			ruleScene = new RuleScene;
			ruleScene->Initialize();
		}
		break;

	case Scene::kRule:
		if(ruleScene->IsFinished()){
			// シーン変更
			scene = Scene::kGame;
			delete ruleScene;
			ruleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;

	case Scene::kGame:

		if(gameScene->IsFinished()){
			// ★ここが変更ポイント★

			// 1. ゲームシーンを消す前に「クリアした？」と結果を聞く
			// (GameScene.h に IsClear() を作っておく必要があります)
			bool isGameClear = gameScene->IsClear();

			// 2. 古いシーンを削除
			delete gameScene;
			gameScene = nullptr;

			// 3. 結果によって行き先を変える
			if(isGameClear){
				// クリアした → エンディングへ
				scene = Scene::kEnd;
				endScene = new EndScene;
				endScene->Initialize();
			} else{
				// 死んで終わった → もう一度ゲームシーンへ (リトライ)
				scene = Scene::kGame;
				gameScene = new GameScene;
				gameScene->Initialize();
			}
		}
		break;
	case Scene::kEnd:
		if (endScene->IsFinished()) {
			scene = Scene::kTitle;
			delete endScene;
			endScene = nullptr;
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
	case Scene::kRule:
		ruleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kEnd:
		endScene->Update();
		break;
	}
}
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kRule:
		ruleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kEnd:
		endScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2C_03_アンドウ_カナデ_AL4");


	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	Audio* audio = Audio::GetInstance();

	audio->Initialize();

	ParticleManager::GetInstance()->Initialize();

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
	ParticleManager::GetInstance()->Shutdown();

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
