#pragma once
#include "Fade.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class EndScene {
public:
	enum class Phase {
		kFadeIn,
		kMain,
		kFadeOut,
	};

	~EndScene();

	void Initialize();

	void Update();

	void Draw();

	bool IsFinished() const { return finished_; }

	void PlayBgm();
	void PlaySe();

private:
	static inline const float kTimeTitleMove = 2.0f;

	Camera camera_;
	WorldTransform worldTransformTitle_;
	WorldTransform worldTransformPlayer_;

	Model* modelPlayer_ = nullptr;
	Model* modelTitle_ = nullptr;

	float counter_ = 0.0f;
	bool finished_ = false;

	Fade* fade_ = nullptr;

	Phase phase_ = Phase::kFadeIn;

	uint32_t bgmDataHandle_;
	uint32_t bgmHandle_;
	bool isPlayBgm_ = false;

	uint32_t seDataHandle_;
	uint32_t seHandle_;
	bool isPlaySe_ = false;

};
