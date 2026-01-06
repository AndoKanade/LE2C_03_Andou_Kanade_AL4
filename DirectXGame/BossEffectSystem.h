#pragma once
#include "ParticleManager.h"
#include "Math.h"
#include <vector>

// 1粒のパーティクルデータ
struct BossParticle{
	KamataEngine::WorldTransform worldTransform; // ★追加: ここに持たせる！
	Vector3 velocity;
	float lifeTimer;
	float maxLife;
	bool isActive;
	KamataEngine::ObjectColor color;
	// position や scale は worldTransform の中にあるので削除してもいいですが、
	// 計算用に残しておいてもOKです。今回は混乱を避けるため既存変数はそのままにします。
	Vector3 position;
	float scale;
};
class BossEffectSystem : public EffectSystemBase{
public:
	// 初期化 (モデルを受け取る)
	void Initialize(Model* model);

	// 更新 (EffectSystemBaseのオーバーライド)
	void Update(float deltaTime) override;

	// 描画 (EffectSystemBaseのオーバーライド)
	void Draw(Camera* camera) override;

	// エフェクト発生 (GameSceneからこれを呼ぶ)
	void Spawn(const Vector3& centerPos);

private:
	// パーティクルの最大数
	static const int kMaxParticles = 50;
	std::array<BossParticle,kMaxParticles> particles_;
	// 描画用モデル
	Model* model_ = nullptr;
	int nextIndex_ = 0;
};