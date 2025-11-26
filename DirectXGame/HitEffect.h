#include <KamataEngine.h>
#include <cstdint>

#pragma once
class HitEffect {
public:
	enum class State {
		kSpread, // 拡大中
		kFade,   // フェードアウト中
		kDead    // 死亡
	};

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }
	static HitEffect* Create(const KamataEngine::Vector3& position);

	void Update();

	void Draw();

	bool IsDead() const { return state_ == State::kDead; }

private:
	HitEffect() = default;

	void Initialize(const KamataEngine::Vector3& position);

	static inline const uint32_t kSpreadTime = 10;

	static inline const uint32_t kFadeTime = 20;

	static inline const uint32_t kLifetime = kSpreadTime + kFadeTime;

	static KamataEngine::Model* model_;
	static KamataEngine::Camera* camera_;

	static const inline uint32_t kellipseEffectNum = 2;

	std::array<KamataEngine::WorldTransform, kellipseEffectNum> ellipseWorldTransforms_;

	KamataEngine::WorldTransform circleWorldTransform_;
	State state_ = State::kSpread;

	uint32_t counter_ = 0;
	KamataEngine::ObjectColor objectColor_;
};