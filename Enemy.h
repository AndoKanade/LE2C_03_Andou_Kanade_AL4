#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;
class Player;

class Enemy {

public:
	enum class Behavior {
		kIdle = -1,
		kWalk,
		kDeath,
	};

	void Initialize(Model* model, Camera* camera, const Vector3& position);

	void Update();

	void Draw();

	AABB GetAABB();

	Vector3 GetWorldPosition();

	void OnCollision(const Player* player);

	bool IsDead() const { return isDead_; }

	void BehaviorRootUpdate();

	void BehaviorDeathUpdate();

	void BehaviorRootInitialize();

	void BehaviorDeathInitialize();

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

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	bool isDead_ = false;

	Behavior behavior_ = Behavior::kWalk;

	Behavior behaviorRequest_ = Behavior::kIdle;
};