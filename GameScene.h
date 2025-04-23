#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

// ゲームシーン
class GameScene {
public:
	~GameScene();
	void Initialize();

	void Update();

	void Draw();

private:
	uint32_t textureHandle_ = 0;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	std::vector<WorldTransform*> worldTransformBlocks_;
	KamataEngine::Camera camera_;

	Player* player_ = nullptr;
};