#include "HitEffect.h"
#include "Math.h"
#include <cassert>
#include <numbers>
#include <random>

using namespace KamataEngine;

KamataEngine::Model* HitEffect::model_ = nullptr;
KamataEngine::Camera* HitEffect::camera_ = nullptr;

HitEffect* HitEffect::Create(const KamataEngine::Vector3& position) {

	HitEffect* instance = new HitEffect();
	assert(instance);

	instance->Initialize(position);

	return instance;
}

void HitEffect::Initialize(const KamataEngine::Vector3& position) {

	circleWorldTransform_.translation_ = position;
	circleWorldTransform_.Initialize();
}

void HitEffect::Update() { WorldTransformUpdate(circleWorldTransform_); }

void HitEffect::Draw() { model_->Draw(circleWorldTransform_, *camera_, &objectColor_); }
