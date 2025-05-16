#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Math.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>

// ゲームシーン
class GameScene {
public:
	~GameScene();
	void Initialize();

	void Update();

	void Draw();

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	bool isDebugCameraActive_ = false;

	KamataEngine::WorldTransform worldTransform_;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;
	KamataEngine::Camera camera_;
	DebugCamera* debugCamera_ = nullptr;

	Player* player_ = nullptr;
	Skydome* skydome_ = nullptr;
	Model* modelSkydome_ = nullptr;
	Model* playerModel_ = nullptr;

	MapChipField* mapChipField_;
	void GenerateBlocks();
};
