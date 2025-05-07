#pragma once
#include "KamataEngine.h"
#include "Math.h"
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

	//	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	bool isDebugCameraActive_ = false;


	KamataEngine::WorldTransform worldTransform_;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;
	KamataEngine::Camera camera_;
	DebugCamera* debugCamera_ = nullptr;

	// Player* player_ = nullptr;
};