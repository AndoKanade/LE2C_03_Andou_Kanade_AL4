#include "GameScene.h"
#include "ImGui.h"
using namespace KamataEngine;

GameScene::~GameScene() { delete model_; }

void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く

	textureHandle_ = TextureManager::Load("ashe.jpg");
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();

	PrimitiveDrawer::GetInstance()->SetViewProjection(&camera_);

	player_ = new Player();
	player_->Initialize(model_, textureHandle_, &camera_);
}

void GameScene::Update() {
	// ここにインゲームの更新処理を書く
	player_->Update();
}

void GameScene::Draw() {
	// ここにインゲームの描画処理を書く
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());
	player_->Draw();
	Model::PostDraw();
}
