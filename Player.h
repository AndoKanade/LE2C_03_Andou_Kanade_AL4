#pragma once
#include "Math.h"
#include <KamataEngine.h>
#include <algorithm>
#include <numbers>
#include <string>

using namespace KamataEngine;

class Player {

public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_;
	Vector3 velocity_;
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 1.0f;
	static inline const float kLimitRunSpeed = 2.0f;

	enum class LRDirection {
		kLeft,
		kRight,
	};

	LRDirection lrDirection_ = LRDirection::kRight;

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	static inline const float lTimeTurn = 0.3f;
};
