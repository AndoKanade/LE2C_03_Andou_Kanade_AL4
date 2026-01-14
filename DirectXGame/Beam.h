#pragma once
#include "KamataEngine.h"
#include "Math.h"

class Beam{
public:
	void Initialize(Model* model,const Vector3& position,const Vector3& velocity);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const{ return isDead_; }
	void OnCollision(); // 何かに当たった時

	void SetIsEnemy(bool isEnemy){ isEnemy_ = isEnemy; }
	bool IsEnemy() const{ return isEnemy_; }

	// 当たり判定用
	Vector3 GetWorldPosition() const{ return worldTransform_.translation_; }
	float GetRadius() const{ return 0.5f; } // 判定半径

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	ObjectColor objectColor_; // 色変更用

	Vector3 velocity_;
	int lifeTimer_ = 120; // 寿命(フレーム数)
	bool isDead_ = false;

	bool isEnemy_ = false;
};
