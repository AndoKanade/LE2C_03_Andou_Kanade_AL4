#pragma once
#include "Fade.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class RuleScene{
public:
	// フェーズ管理（タイトルと同じ）
	enum class Phase{
		kFadeIn,
		kMain,
		kFadeOut,
	};

	~RuleScene();

	void Initialize();

	void Update();

	void Draw();

	bool IsFinished() const{ return finished_; }

private:
	// 画像ハンドル
	uint32_t textureHandle_ = 0;
	// スプライト
	Sprite* sprite_ = nullptr;

	bool finished_ = false;

	Fade* fade_ = nullptr;

	Phase phase_ = Phase::kFadeIn;
};