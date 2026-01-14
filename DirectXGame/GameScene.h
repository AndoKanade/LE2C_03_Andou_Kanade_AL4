#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "HitEffect.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "skydome.h"
#include <iostream>

#include <vector>

class Beam{
public:
	void Initialize(Model* model,const Vector3& position,const Vector3& velocity);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const{ return isDead_; }
	void OnCollision(); // 何かに当たった時

	// 当たり判定用
	Vector3 GetWorldPosition() const{ return worldTransform_.translation_; }
	// 簡易的な球判定用の半径
	float GetRadius() const{ return 0.5f; }

	ObjectColor objectColor_;


private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Vector3 velocity_;
	int lifeTimer_ = 120; // 2秒で消える
	bool isDead_ = false;
};


// ゲームシーンクラス
class GameScene{
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

	bool IsFinished() const{ return finished_; }

	// エフェクトを生成
	void CreateEffect(const Vector3& position);

private:
	enum class Phase{
		kFadeIn,  // フェードイン 02_13 28枚目で追加
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト 02_13 28枚目で追加
	};

	Phase phase_;

	Model* modelBeam_ = nullptr; // ビームのモデル
	std::list<Beam*> beams_;     // ビームのリスト


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
	KamataEngine::Model* modelEnemy2_ = nullptr;
	KamataEngine::Model* modelEnemy3_ = nullptr;

	std::list<Enemy*> enemies_;
	// GameScene.h の private メンバ変数に追加
	Model* modelBoss_ = nullptr; // ★追加
	DeathParticles* deathParticles_ = nullptr;

	Model* modelDeathEffect_ = nullptr;

	bool finished_ = false;
	Fade* fade_ = nullptr;

	Model* modelAttack_ = nullptr;

	std::list<HitEffect*> hitEffects_;

	Model* modelParticle_ = nullptr;

	Model* modelArrow_ = nullptr;
};