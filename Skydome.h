#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;

class Skydome {
public:
	void Initialize(KamataEngine::Camera* camera, KamataEngine::Model* model);
	void Update();
	void Draw();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera* camera_;
	KamataEngine::Model* model_ = nullptr;
};
