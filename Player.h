#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;
class Player {
public:
	void Initialize(Model* model, Camera* camera);
	void Update();
	void Draw();

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_;
};
