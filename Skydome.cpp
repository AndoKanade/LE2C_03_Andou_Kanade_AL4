#include "Skydome.h"
#include <assert.h>

void Skydome::Initialize(Camera* camera, Model* model) {

	assert(model);

	worldTransform_.Initialize();
	camera_ = camera;
	model_ = model;
}

void Skydome::Update() { worldTransform_.TransferMatrix(); }

void Skydome::Draw() { model_->Draw(worldTransform_, *camera_); }
