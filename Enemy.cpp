#include "Enemy.h"
#include "Math.h"
#include <cassert>
#include <numbers>

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);

	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	velocity_ = {-kWalkSpeed, 0, 0};
	walkTimer = 0.0f;
}

void Enemy::Update() {

	worldTransform_.translation_ += velocity_;
	walkTimer += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);
	float startDegree = -30.0f; // 初期の角度
	float endDegree = 30.0f;    // 最後の角度
	float degree = startDegree + (endDegree - startDegree) * (param + 1.0f) / 2.0f;
	worldTransform_.rotation_.x = degree * (std::numbers::pi_v<float> / 180.0f); // ラジアン変換

	WorldTransformUpdate(worldTransform_);
}


void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }