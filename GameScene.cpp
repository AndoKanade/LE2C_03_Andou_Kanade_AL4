#include "GameScene.h"
using namespace KamataEngine;

void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く

	textureHandle_ = TextureManager::Load("mario.jpg");
	sprite_ = Sprite::Create(textureHandle_, {100, 50});

	delete sprite_;
}

void GameScene::Update() {
	// ここにインゲームの更新処理を書く
}

void GameScene::Draw() {
	// ここにインゲームの描画処理を書く
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Sprite::PreDraw(dxCommon->GetCommandList());

	sprite_->Draw();

	Sprite::PostDraw();
}
