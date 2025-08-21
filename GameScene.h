#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "HitEffect.h"
#include "Item.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "skydome.h"
#include <iostream>

#include <vector>

// ゲームシーンクラス
class GameScene {
public:
	~GameScene();
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	void GenerateBlocks();

	void CheckAllCollisions();

	bool IsFinished() const { return finished_; }

	// エフェクトを生成
	void CreateEffect(const Vector3& position);

private:
	enum class Phase {
		kFadeIn,  // フェードイン 02_13 28枚目で追加
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト 02_13 28枚目で追加
	};

	Phase phase_;

	void ChangePhase();

	uint32_t textureHandle_ = 0;
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera camera_;
	uint32_t soundDataHandle_ = 0;
	uint32_t voiceHandle_ = 0;

	//  プレイヤー
	Player* player_ = nullptr;
	// プレイヤーモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	// ブロックモデル
	KamataEngine::Model* modelBlock_ = nullptr;
	Player* item_ = nullptr;
	KamataEngine::Model* modelItem_ = nullptr;
	std::vector<Item*> items_; // 敵と同じように管理

	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Skydome* skydome_ = nullptr;
	Model* modelSkydome_ = nullptr;

	MapChipField* mapChipField_;

	CameraController* cameraController_ = nullptr;

	KamataEngine::Model* modelEnemy_ = nullptr;

	std::list<Enemy*> enemies_;
	DeathParticles* deathParticles_ = nullptr;

	Model* modelDeathEffect_ = nullptr;

	bool finished_ = false;
	Fade* fade_ = nullptr;

	Model* modelAttack_ = nullptr;

	std::list<HitEffect*> hitEffects_;

	Model* modelParticle_ = nullptr;
};