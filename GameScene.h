#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "HitEffect.h"
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

	void GenerateBlocks();

	void CheckAllCollisions();

	bool IsFinished() const { return finished_; }

	void CreateEffect(const KamataEngine::Vector3& position);

private:
	enum class Phase {
		kFadeIn,
		kPlay,
		kDeath,
		kFadeOut,
	};
	Phase phase_;

	void ChangePhase();

	KamataEngine::Sprite* sprite_ = nullptr;
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
	Model* modelPlayer_ = nullptr;
	Model* modelPlayerAttack_ = nullptr;
	Model* modelParticle_ = nullptr;
	Model* modelDeathParticle_ = nullptr;

	MapChipField* mapChipField_;

	CameraController* cameraController_ = nullptr;
	Enemy* enemy_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;

	std::list<Enemy*> enemies_;
	DeathParticles* deathParticles_ = nullptr;

	std::list<HitEffect*> hitEffects_;

	bool finished_ = false;

	Fade* fade_ = nullptr;
};
