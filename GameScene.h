#pragma once
#include "KamataEngine.h"

// ゲームシーン
class GameScene {
public:
	uint32_t textureHandle_ = 0;
	
	KamataEngine::Sprite* sprite_ = nullptr;

	void Initialize();

	void Update();

	void Draw();

	
};