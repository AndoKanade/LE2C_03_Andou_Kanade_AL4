#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;

class Skydome {
public:
	void Initialize(Camera* camera,Model* model);
	void Update();
	void Draw();

private:
	WorldTransform worldTransform_;

	Camera* camera_;
	Model* model_ = nullptr;
};
