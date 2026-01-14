#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

class MapChipField;
class Enemy;

class Player{
public:
	// =========================================================
	// 列挙型定義
	// =========================================================

	// 左右の向き
	enum class LRDirection{
		kRight,
		kLeft,
	};

	// 矩形の角
	enum Corner{ kRightBottom,kLeftBottom,kRightTop,kLeftTop,kNumCorner };

	// 行動ステート
	enum class Behavior{
		kUnknown = -1,
		kRoot,   // 通常状態
		kAttack, // 攻撃中（突進・ソード）
		kShot,   // 射撃中（ビーム硬直）
	};

	// 攻撃フェーズ
	enum class AttackPhase{
		kUnknown = -1,
		kAnticipation, // 予備動作
		kAction,       // 前進動作
		kRecovery,     // 余韻動作
	};

	// 能力の種類
	enum class AbilityType{
		Sword, // 近距離（既存の突進攻撃）
		Beam   // 遠距離（新規）
	};

	// =========================================================
	// パブリック関数 (外部から呼ぶもの)
	// =========================================================

	// 初期化・更新・描画
	void Initialize(Model* model,Model* modelAttack,Camera* camera,const Vector3& position);
	void Update();
	void Draw();

	// --- ゲッター・セッター ---
	const WorldTransform& GetWorldTransform() const{ return worldTransform_; }
	const Vector3& GetVelocity() const{ return velocity_; }
	Vector3 GetWorldPosition() const;
	AbilityType GetAbilityType() const{ return abilityType_; } // 現在の能力取得
	AABB GetAABB();

	void SetMapChipField(MapChipField* mapChipField){ mapChipField_ = mapChipField; }
	// (ポインタのポインタを返すのは稀ですが、既存コード維持)
	void GetMapChipField(MapChipField** mapChipField){ *mapChipField = mapChipField_; }

	// --- 状態判定・フラグ処理 ---
	bool IsDead() const{ return isDead_; }
	bool IsAttack() const{ return behavior_ == Behavior::kAttack && attackPhase_ == AttackPhase::kAction; }
	bool IsCollisionDisabled() const{ return isCollisionDisabled_; }

	// テスト用フラグ
	bool CanICrear() const{ return canICrear; }
	bool CanICrear1() const{ return canICrear1; }

	// 衝突コールバック
	void OnCollision(const Enemy* enemy);

	// 射撃リクエスト確認（呼び出すとフラグが折れる）
	bool IsShotBeam(){
		if(isShotBeamRequest_){
			isShotBeamRequest_ = false;
			return true;
		}
		return false;
	}

	// 吸い込み中かどうか
	bool IsInhaling() const{ return isInhaling_; }

	// 弾を持っているか（満腹か）
	bool HasAmmo() const{ return hasAmmo_; }

	// 弾を飲み込んだときに呼ぶ関数
	void CatchAmmo(){ hasAmmo_ = true; }

	// 吸い込み判定エリア（口元の判定）を取得
	AABB GetInhaleArea();


private:
	// =========================================================
	// プライベート関数 (内部処理用)
	// =========================================================

	// --- 行動ステート別更新 ---
	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();

	void BehaviorShotInitialize();
	void BehaviorShotUpdate();

	// --- 移動・物理演算 ---
	void InputMove(); // 入力処理

	// --- マップ衝突判定 ---
	struct CollisionMapInfo{
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		Vector3 move;
	};
	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	void UpdateOnGround(const CollisionMapInfo& info);
	void UpdateOnWall(const CollisionMapInfo& info);

	// ユーティリティ
	Vector3 CornerPosition(const Vector3& center,Corner corner);


	// =========================================================
	// メンバ変数
	// =========================================================

	// --- システム・モデル ---
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Model* modelAttack_ = nullptr;
	WorldTransform worldTransformAttack_;
	KamataEngine::ObjectColor objectColor_; // 色変更用
	Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0u;

	// --- ステージ情報 ---
	MapChipField* mapChipField_ = nullptr;

	// --- 物理・移動パラメータ ---
	Vector3 velocity_ = {};
	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	bool onGround_ = true;
	bool isHovering_ = false;

	// --- 状態管理 ---
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;
	bool canICrear = false;
	bool canICrear1 = false;
	int counter = 0;

	// --- ステートマシン ---
	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 攻撃パラメータ
	AttackPhase attackPhase_ = AttackPhase::kUnknown;
	uint32_t attackParameter_ = 0;

	// 能力・射撃パラメータ
	AbilityType abilityType_ = AbilityType::Sword;
	bool isShotBeamRequest_ = false;
	uint32_t shotTimer_ = 0;

	// =========================================================
	// 定数 (調整用パラメータ)
	// =========================================================

	// キャラクターサイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.04f;
	static inline const float kGroundSearchHeight = 0.06f;

	// 移動・ジャンプ
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.05f;
	static inline const float kLimitRunSpeed = 0.3f;
	static inline const float kTimeTurn = 0.3f;
	static inline const float kJumpAcceleration = 20.0f;
	static inline const float kGravityAcceleration = 0.98f;
	static inline const float kLimitFallSpeed = 0.5f;
	static inline const float kAttenuationLanding = 0.0f;
	static inline const float kAttenuationWall = 0.2f;

	// ホバリング（仮）
	const float kHoverAcceleration = kGravityAcceleration * 1.5f;
	const float kAirControlAcceleration = kAcceleration * 0.5f;
	const float kLimitAirSpeed = kLimitRunSpeed * 0.7f;
	const float kAirAttenuation = 0.02f;
	const float kHoverImpulse = 25.0f;
	const float kLimitHoverImpulseSpeed = kJumpAcceleration / 60.0f;

	// アクションフレーム数
	static inline const uint32_t kAnticipationTime = 8;
	static inline const uint32_t kActionTime = 5;
	static inline const uint32_t kRecoveryTime = 12;
	static inline const uint32_t kShotTime = 20; // 射撃硬直

	bool isInhaling_ = false; // 吸い込みキーを押しているか
	bool hasAmmo_ = false;    // 敵の弾を保持しているか

};