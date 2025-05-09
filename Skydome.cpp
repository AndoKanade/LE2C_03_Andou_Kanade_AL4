#include "Skydome.h"

void Skydome::Initialize(KamataEngine::Camera* camera, KamataEngine::Model* model) {
	worldTransform_.Initialize();
	camera_ = camera;
	model_ = model;
}

void Skydome::Update() {}

void Skydome::Draw() { model_->Draw(worldTransform_, *camera_); }
