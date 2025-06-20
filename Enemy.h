#pragma once

#include "KamataEngine.h"

using namespace KamataEngine;
class Enemy {

public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	void Update();

	void Draw();

private:
	WorldTransform worldTransform_;

	Model* model_ = nullptr;

	Camera* camera_ = nullptr;

	static inline const float kWalkSpeed = 0.02f;

	Vector3 velocity_ = {};

	static inline const float kWalkMotionAngleStart = 0.0f;

	static inline const float kWalkMotionAngleEnd = 30.0f;

	static inline const float kWalkMotionTime = 1.0f;

	float walkTimer = 0.0f;
};