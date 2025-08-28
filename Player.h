#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

class MapChipField;
class Enemy;

class Player {
public:
	enum class LRDirection {
		kRight,
		kLeft,
	};

	enum Corner { kRightBottom, kLeftBottom, kRightTop, kLeftTop, kNumCorner };

	enum class Behavior {
		kUnknown = -1,
		kRoot,   // 通常状態
		kAttack, // 攻撃中
	};

	enum class AttackPhase {
		kUnknown = -1, // 無効な状態

		kAnticipation, // 予備動作
		kAction,       // 前進動作
		kRecovery,     // 余韻動作
	};

	void Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position);

	void Update();

	void Draw();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void GetMapChipField(MapChipField** mapChipField) { *mapChipField = mapChipField_; }

	Vector3 GetWorldPosition() const;

	AABB GetAABB();

	void OnCollision(const Enemy* enemy);

	bool IsDead() const { return isDead_; }

	bool CanICrear() const { return canICrear; }
	bool CanICrear1() const { return canICrear1; }

	void BehaviorRootUpdate();

	void BehaviorAttackUpdate();

	void BehaviorRootInitialize();

	void BehaviorAttackInitialize();

	bool IsAttack() const { return behavior_ == Behavior::kAttack && attackPhase_ == AttackPhase::kAction; }

	// 02_15
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;

	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.05f;
	static inline const float kLimitRunSpeed = 0.3f;
	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;
	bool onGround_ = true;
	static inline const float kJumpAcceleration = 20.0f;
	static inline const float kGravityAcceleration = 0.98f;
	static inline const float kLimitFallSpeed = 0.5f;

	MapChipField* mapChipField_ = nullptr;
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.04f;

	void InputMove();
	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		Vector3 move;
	};
	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);
	Vector3 CornerPosition(const Vector3& center, Corner corner);

	void UpdateOnGround(const CollisionMapInfo& info);

	void UpdateOnWall(const CollisionMapInfo& info);

	static inline const float kAttenuationLanding = 0.0f;
	static inline const float kGroundSearchHeight = 0.06f;
	static inline const float kAttenuationWall = 0.2f;
	bool isDead_ = false;

	Behavior behavior_ = Behavior::kRoot;

	Behavior behaviorRequest_ = Behavior::kUnknown;

	uint32_t attackParameter_ = 0;

	AttackPhase attackPhase_ = AttackPhase::kUnknown;

	static inline const uint32_t kAnticipationTime = 8;
	static inline const uint32_t kActionTime = 5;
	static inline const uint32_t kRecoveryTime = 12;
	Model* modelAttack_ = nullptr;
	WorldTransform worldTransformAttack_;

	bool isCollisionDisabled_ = false; // 衝突無効化

	bool canICrear = false;
	bool canICrear1 = false;

	int counter;
};