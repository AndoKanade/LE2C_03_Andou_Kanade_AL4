#include "GameScene.h"
using namespace KamataEngine;

// GameScene::~GameScene() {
//	delete sprite_;
//	delete model_;
// delete debugCamera_;
// }

void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く

	textureHandle_ = TextureManager::Load("ashe.jpg");
	sprite_ = Sprite::Create(textureHandle_, {100, 50});
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();

	soundDataHandle_ = Audio::GetInstance()->LoadWave("fanfare.wav");
	Audio::GetInstance()->PlayWave(soundDataHandle_);
	voiceHandle_ = Audio::GetInstance()->PlayWave(soundDataHandle_, true);

	// PrimitiveDrawer::GetInstance()->SetCamera(&camera_);
	debugCamera_ = new DebugCamera(1280, 720);

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());
}

void GameScene::Update() {
	// ここにインゲームの更新処理を書く
	/*
	Vector2 pos = sprite_->GetPosition();
	pos.x += 2.0f;
	pos.y += 1.0f;
	sprite_->SetPosition(pos);
	*/
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		Audio::GetInstance()->StopWave(voiceHandle_);
	}

	debugCamera_->Update();

#ifdef _DEBUG
	ImGui::Begin("Debug1");
	ImGui::InputFloat3("InputFloat3", inputFloat3);
	ImGui::SliderFloat3("SliderFloat3", inputFloat3, 0.0f, 1.0f);
	ImGui::End();
	ImGui::ShowDemoWindow();
#endif
}

void GameScene::Draw() {
	// ここにインゲームの描画処理を書く
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Sprite::PreDraw(dxCommon->GetCommandList());
	sprite_->Draw();
	Sprite::PostDraw();

	Model::PreDraw(dxCommon->GetCommandList());
	model_->Draw(worldTransform_, debugCamera_->GetCamera(), textureHandle_);
	Model::PostDraw();

	// PrimitiveDrawer::GetInstance()->DrawLine3D({0, 0, 0}, {0, 10, 0}, {1.0f, 0.0f, 0.0f, 1.0f});
}
