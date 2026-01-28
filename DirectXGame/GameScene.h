#pragma once
#include "KamataEngine.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "HitEffect.h"
#include "MapChipField.h"
#include "Player.h"
#include "skydome.h"

#include <vector>
#include <list> 
#include "Beam.h"

using namespace KamataEngine;

// ==========================================
// ゲームシーンクラス
// ==========================================
class GameScene{
public:
	// コンストラクタ・デストラクタ
	GameScene(){} // 明示的に書く場合
	~GameScene();

	// --- メインループ ---
	void Initialize();
	void Update();
	void Draw();

	// --- 状態取得 ---
	bool IsFinished() const{ return finished_; }

	bool IsClear() const{ return isClear_; }

	// エフェクト生成ヘルパー
	void CreateEffect(const Vector3& position);
private:
	// --- 内部メソッド ---

	// フェーズ切り替えチェック
	void ChangePhase();

	// マップブロック生成
	void GenerateBlocks();

	void GenerateEnemies();

	// 全衝突判定
	void CheckAllCollisions();

	// ★追加: ゲームオブジェクト一括更新 (コード整理用)
	void UpdateGameObjects();


	// --- メンバ変数 ---

	// 1. システム・カメラ
	Camera camera_;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;
	CameraController* cameraController_ = nullptr;

	Fade* fade_ = nullptr;
	bool finished_ = false;

	enum class Phase{
		kFadeIn,  // フェードイン
		kPlay,    // プレイ中
		kDeath,   // 死亡演出
		kFadeOut, // フェードアウト
	};
	Phase phase_ = Phase::kFadeIn;

	// 2. ステージ・背景
	Skydome* skydome_ = nullptr;
	Model* modelSkydome_ = nullptr;

	MapChipField* mapChipField_ = nullptr;
	Model* modelBlock_ = nullptr;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// 3. プレイヤー
	Player* player_ = nullptr;
	Model* modelPlayer_ = nullptr;
	Model* modelAttack_ = nullptr; // 攻撃エフェクト用

	// 4. 敵キャラクター
	std::list<Enemy*> enemies_;
	Model* modelEnemy_ = nullptr; // カカシ
	Model* modelBoss_ = nullptr;  // ボス

	// 5. 弾(ビーム)
	std::list<Beam*> beams_;
	Model* modelBeam_ = nullptr;

	// 6. パーティクル・エフェクト
	DeathParticles* deathParticles_ = nullptr;
	Model* modelDeathEffect_ = nullptr;

	std::list<HitEffect*> hitEffects_;
	Model* modelParticle_ = nullptr; // 汎用パーティクルモデル

	// 7. UI・その他
	uint32_t textureHandle_ = 0;
	Sprite* sprite_ = nullptr;
	Model* modelArrow_ = nullptr;

	// 汎用モデル（必要なら使用）
	Model* model_ = nullptr;

	// 音声ハンドル
	uint32_t soundDataHandle_ = 0;
	uint32_t voiceHandle_ = 0;

	bool isClear_ = false;
};