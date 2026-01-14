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

using namespace KamataEngine;

// ==========================================
// ビームクラス (簡易定義)
//本来は別ファイルにするのがベストですが、ここにまとめておきます
// ==========================================
class Beam{
public:
	void Initialize(Model* model,const Vector3& position,const Vector3& velocity);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const{ return isDead_; }
	void OnCollision(); // 何かに当たった時

	// 当たり判定用
	Vector3 GetWorldPosition() const{ return worldTransform_.translation_; }
	float GetRadius() const{ return 0.5f; } // 判定半径

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	ObjectColor objectColor_; // 色変更用

	Vector3 velocity_;
	int lifeTimer_ = 120; // 寿命(フレーム数)
	bool isDead_ = false;
};

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

	// エフェクト生成ヘルパー
	void CreateEffect(const Vector3& position);
private:
	// --- 内部メソッド ---

	// フェーズ切り替えチェック
	void ChangePhase();

	// マップブロック生成
	void GenerateBlocks();

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
};