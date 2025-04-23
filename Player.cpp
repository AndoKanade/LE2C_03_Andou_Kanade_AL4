#include "Player.h"

void Player::Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera) {
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;
	worldTransform_.Initialize();
	camera_ = camera;
}
void Player::Update() {
	// プレイヤーの更新処理を書く
	worldTransform_.TransferMatrix();
}
void Player::Draw() {
	// プレイヤーの描画処理を書く
	// モデルの描画

	model_->Draw(worldTransform_, *camera_, textureHandle_);
}
