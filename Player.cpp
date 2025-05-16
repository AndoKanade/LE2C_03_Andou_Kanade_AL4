#include "Player.h"

void Player::Initialize(Model* model, Camera* camera) {
	assert(model);

	model_ = model;

	worldTransform_.Initialize();
	camera_ = camera;
}
void Player::Update() {
	// プレイヤーの更新処理を書く
	/// 3次元のアフェイン変換行列を作成する
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}
void Player::Draw() {
	// プレイヤーの描画処理を書く
	// モデルの描画

	model_->Draw(worldTransform_, *camera_);
}
