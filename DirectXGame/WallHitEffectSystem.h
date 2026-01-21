#pragma once
#include "KamataEngine.h"
#include "ParticleManager.h" // EffectSystemBaseを使うために必要ならインクルード
#include <list>

using namespace KamataEngine;

// 壁ヒット用の一粒
struct WallHitParticle{
	WorldTransform worldTransform;
	ObjectColor objectColor;
	Vector3 velocity;
	int lifeTimer = 0;
};

// ★修正: : public EffectSystemBase を追加して継承させる
class WallHitEffectSystem : public EffectSystemBase{
public:
	// EffectSystemBaseの仮想関数に合わせる必要があるため override を推奨
	void Initialize(Model* model); // 必要に応じて引数調整

	// ★修正: Updateに (float deltaTime) を追加
	void Update(float deltaTime) override;

	void Draw(Camera* camera) override;

	// 発生させる関数
	void Spawn(const Vector3& position);

private:
	std::list<WallHitParticle*> particles_;
	Model* model_ = nullptr;
};