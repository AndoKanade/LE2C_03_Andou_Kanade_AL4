#define NOMINMAX

#include "Player.h"
#include "JumpSystem.h"
#include "MapChipField.h"
#include "Math.h"
#include "ParticleManager.h"
#include <algorithm>
#include <cassert>
#include <numbers>

// =================================================================
// 初期化処理
// =================================================================
void Player::Initialize(Model* model,Model* modelAttack,Camera* camera,const Vector3& position){
	assert(model);

	// --- モデルとカメラのセット ---
	model_ = model;
	modelAttack_ = modelAttack;
	camera_ = camera;

	// --- 座標と回転の初期化 ---
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 攻撃用モデル（剣など）の座標同期用
	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	// --- パーティクルシステムの取得 ---
	JumpSystem* jump = ParticleManager::GetInstance()->GetJumpSystem();
	if(jump){
		jump->Initialize(model_,textureHandle_);
	}

	// --- パラメータ初期化 ---
	objectColor_.Initialize();

	// ★追加: 状態のリセット
	isInhaling_ = false;
	hasAmmo_ = false;
}

// =================================================================
// 更新処理 (メインループ)
// =================================================================
void Player::Update(){

	// --- 行動遷移リクエストの処理 ---
	if(behaviorRequest_ != Behavior::kUnknown){
		behavior_ = behaviorRequest_;

		switch(behavior_){
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		case Behavior::kShot: // ビーム撃ち始め
			BehaviorShotInitialize();
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	// --- 現在の振る舞いごとの更新 ---
	switch(behavior_){
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate(); // 通常移動
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate(); // ※今回は使わないかもですが残しておきます
		break;
	case Behavior::kShot:
		BehaviorShotUpdate(); // 吐き出し攻撃
		break;
	}

	// --- 色の反映（状態に合わせて色を変える） ---
	if(hasAmmo_){
		// 弾を持っている時：オレンジ（満腹）
		objectColor_.SetColor({1.0f, 0.5f, 0.0f, 1.0f});
	} else if(isInhaling_){
		// 吸い込み中：青（吸い込み）
		objectColor_.SetColor({0.5f, 0.5f, 1.0f, 1.0f});
	} else{
		// 通常時：白
		objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}

	// --- 行列更新 ---
	WorldTransformUpdate(worldTransform_);
	WorldTransformUpdate(worldTransformAttack_);
}

// =================================================================
// 描画処理
// =================================================================
void Player::Draw(){
	// プレイヤー本体の描画
	model_->Draw(worldTransform_,*camera_,&objectColor_);
}

// =================================================================
// 振る舞い: 通常 (Root)
// =================================================================
void Player::BehaviorRootInitialize(){}

void Player::BehaviorRootUpdate(){

	// 入力による移動・ジャンプ処理
	InputMove();

	// --- マップ衝突判定 ---
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;

	if(collisionMapInfo.ceiling){
		velocity_.y = 0;
	}
	UpdateOnWall(collisionMapInfo);
	UpdateOnGround(collisionMapInfo);

	// --- 旋回アニメーション ---
	if(turnTimer_ > 0.0f){
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f),0.0f);
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f,
											 std::numbers::pi_v<float> *3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY,turnFirstRotationY_,turnTimer_ / kTimeTurn);
	}

	// =========================================================
	// ★変更: 吸い込み・吐き出しアクション
	// =========================================================

	// まだ弾を持っていない場合 -> 「吸い込み」が可能
	if(!hasAmmo_){
		if(Input::GetInstance()->PushKey(DIK_SPACE)){
			// スペースキーを押している間、吸い込み状態にする
			isInhaling_ = true;
		} else{
			isInhaling_ = false;
		}
	}
	// すでに弾を持っている場合 -> 「吐き出し」が可能
	else{
		isInhaling_ = false; // 満腹なので吸い込めない

		// スペースキーで発射！
		if(Input::GetInstance()->TriggerKey(DIK_SPACE)){
			behaviorRequest_ = Behavior::kShot; // 射撃ステートへ遷移
			hasAmmo_ = false; // 弾を消費して空っぽに戻る
		}
	}
}

// =================================================================
// 振る舞い: ソード攻撃 (今回は未使用に近いが残しておく)
// =================================================================
void Player::BehaviorAttackInitialize(){
	attackParameter_ = 0;
	velocity_ = {};
	attackPhase_ = AttackPhase::kAnticipation;
}

void Player::BehaviorAttackUpdate(){
	// 既存コード維持（もしソード攻撃を使いたくなったとき用）
	const Vector3 attackVelocity = {0.8f, 0.0f, 0.0f};
	Vector3 velocity{};
	attackParameter_++;

	switch(attackPhase_){
	case AttackPhase::kAnticipation:
	default:
	{
		velocity = {};
		float t = static_cast<float>(attackParameter_) / kAnticipationTime;
		worldTransform_.scale_.z = EaseOut(1.0f,0.3f,t);
		worldTransform_.scale_.y = EaseOut(1.0f,1.6f,t);

		if(attackParameter_ >= kAnticipationTime){
			attackPhase_ = AttackPhase::kAction;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kAction:
	{
		if(lrDirection_ == LRDirection::kRight){
			velocity = +attackVelocity;
		} else{
			velocity = -attackVelocity;
		}
		float t = static_cast<float>(attackParameter_) / kActionTime;
		worldTransform_.scale_.z = EaseOut(0.3f,1.3f,t);
		worldTransform_.scale_.y = EaseIn(1.6f,0.7f,t);

		if(attackParameter_ >= kActionTime){
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kRecovery:
	{
		velocity = {};
		float t = static_cast<float>(attackParameter_) / kRecoveryTime;
		worldTransform_.scale_.z = EaseOut(1.3f,1.0f,t);
		worldTransform_.scale_.y = EaseOut(0.7f,1.0f,t);

		if(attackParameter_ >= kRecoveryTime){
			behaviorRequest_ = Behavior::kRoot;
		}
		break;
	}
	}

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity;
	CheckMapCollision(collisionMapInfo);
	worldTransform_.translation_ += collisionMapInfo.move;

	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
}

// =================================================================
// 振る舞い: ビーム射撃 (Shot) = 吐き出し攻撃
// =================================================================
void Player::BehaviorShotInitialize(){
	// 足を止める
	velocity_ = {};

	// 硬直時間（吐き出しモーションの時間）
	shotTimer_ = kShotTime;

	// GameScene側で検知して弾を生成するフラグON
	isShotBeamRequest_ = true;
}

void Player::BehaviorShotUpdate(){
	// 重力のみ適用
	velocity_.y += -kGravityAcceleration / 60.0f;
	velocity_.y = std::max(velocity_.y,-kLimitFallSpeed);

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;
	UpdateOnGround(collisionMapInfo);

	shotTimer_--;

	if(shotTimer_ <= 0){
		behaviorRequest_ = Behavior::kRoot;
	}
}

// =================================================================
// ★追加: 吸い込み範囲の取得
// =================================================================

AABB Player::GetInhaleArea(){
	Vector3 pos = GetWorldPosition();
	float range = 4.0f; // 吸い込み範囲の広さ（大きめに設定）

	AABB area;
	// プレイヤーの向いている方向の前方に判定ボックスを作る
	if(lrDirection_ == LRDirection::kRight){
		area.min = {pos.x + 0.5f, pos.y - 1.5f, pos.z - 2.0f};
		area.max = {pos.x + 0.5f + range, pos.y + 1.5f, pos.z + 2.0f};
	} else{
		area.min = {pos.x - 0.5f - range, pos.y - 1.5f, pos.z - 2.0f};
		area.max = {pos.x - 0.5f, pos.y + 1.5f, pos.z + 2.0f};
	}
	return area;
}


// =================================================================
// 入力と物理挙動 (InputMove) - 変更なし
// =================================================================
void Player::InputMove(){

	if(onGround_){
		isHovering_ = false;

		if(Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)){
			Vector3 acceleration = {};

			if(Input::GetInstance()->PushKey(DIK_RIGHT)){
				if(velocity_.x < 0.0f){ velocity_.x *= (1.0f - kAttenuation); }
				acceleration.x += kAcceleration / 60.0f;
				if(lrDirection_ != LRDirection::kRight){
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if(Input::GetInstance()->PushKey(DIK_LEFT)){
				if(velocity_.x > 0.0f){ velocity_.x *= (1.0f - kAttenuation); }
				acceleration.x -= kAcceleration / 60.0f;
				if(lrDirection_ != LRDirection::kLeft){
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ += acceleration;
			velocity_.x = std::clamp(velocity_.x,-kLimitRunSpeed,kLimitRunSpeed);
		} else{
			velocity_.x *= (1.0f - kAttenuation);
		}

		if(std::abs(velocity_.x) <= 0.0001f){
			velocity_.x = 0.0f;
		}

		if(Input::GetInstance()->PushKey(DIK_UP)){
			velocity_ += Vector3(0,kJumpAcceleration / 60.0f,0);

			JumpSystem* jumpSys = ParticleManager::GetInstance()->GetJumpSystem();
			if(jumpSys){
				Vector3 footPos = worldTransform_.translation_;
				footPos.y -= 1.0f;
				jumpSys->Spawn(footPos);
			}
		}
	} else{
		isHovering_ = true;

		if(Input::GetInstance()->TriggerKey(DIK_UP)){
			velocity_.y += kHoverImpulse;
			velocity_.y = std::min(velocity_.y,kLimitHoverImpulseSpeed);
		} else{
			isHovering_ = false;
			velocity_ += Vector3(0,-kGravityAcceleration / 60.0f,0);
			velocity_.y = std::max(velocity_.y,-kLimitFallSpeed);
		}

		if(Input::GetInstance()->PushKey(DIK_RIGHT)){
			velocity_.x += kAirControlAcceleration / 60.0f;
			if(lrDirection_ != LRDirection::kRight){
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		} else if(Input::GetInstance()->PushKey(DIK_LEFT)){
			velocity_.x -= kAirControlAcceleration / 60.0f;
			if(lrDirection_ != LRDirection::kLeft){
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		}

		velocity_.x = std::clamp(velocity_.x,-kLimitAirSpeed,kLimitAirSpeed);
		velocity_.x *= (1.0f - kAirAttenuation);
	}
}

// =================================================================
// 衝突判定系
// =================================================================

void Player::OnCollision(const Enemy* enemy){
	// 攻撃中(吐き出し中)は無敵
	if(IsAttack()){
		return;
	}
	(void)enemy;

	canICrear = true;
	counter++;
	if(canICrear){
		if(counter <= 3000){
			canICrear1 = true;
		}
	}

	// isDead_ = true;
	isCollisionDisabled_ = true;
}

// --- 以下、マップチップ判定処理（変更なし） ---

void Player::CheckMapCollision(CollisionMapInfo& info){
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info){
	if(info.move.y <= 0) return;

	std::array<Vector3,kNumCorner> positionsNew;
	for(uint32_t i = 0; i < positionsNew.size(); ++i){
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
	}

	bool hit = false;
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]).yIndex) == MapChipType::kBlock){
		hit = true;
	}
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]).yIndex) == MapChipType::kBlock){
		hit = true;
	}

	if(hit){
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(
			worldTransform_.translation_ + info.move + Vector3(0,+kHeight / 2.0f,0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex,indexSet.yIndex);

		info.move.y = std::max(0.0f,rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		info.ceiling = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info){
	if(info.move.y >= 0) return;

	std::array<Vector3,kNumCorner> positionsNew;
	for(uint32_t i = 0; i < positionsNew.size(); ++i){
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
	}

	bool hit = false;
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]).yIndex) == MapChipType::kBlock){
		hit = true;
	}
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]).yIndex) == MapChipType::kBlock){
		hit = true;
	}

	if(hit){
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(
			worldTransform_.translation_ + info.move + Vector3(0,-kHeight / 2.0f,0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex,indexSet.yIndex);

		info.move.y = std::min(0.0f,rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		info.landing = true;
	}
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info){
	if(info.move.x <= 0) return;

	std::array<Vector3,kNumCorner> positionsNew;
	for(uint32_t i = 0; i < positionsNew.size(); ++i){
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
	}

	bool hit = false;
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]).yIndex) == MapChipType::kBlock){
		hit = true;
	}
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]).yIndex) == MapChipType::kBlock){
		hit = true;
	}

	if(hit){
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(
			worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f,0,0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex,indexSet.yIndex);

		info.move.x = std::max(0.0f,rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info){
	if(info.move.x >= 0) return;

	std::array<Vector3,kNumCorner> positionsNew;
	for(uint32_t i = 0; i < positionsNew.size(); ++i){
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
	}

	bool hit = false;
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]).yIndex) == MapChipType::kBlock){
		hit = true;
	}
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]).yIndex) == MapChipType::kBlock){
		hit = true;
	}

	if(hit){
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(
			worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f,0,0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex,indexSet.yIndex);

		info.move.x = std::max(0.0f,rect.right - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

Vector3 Player::CornerPosition(const Vector3& center,Corner corner){
	Vector3 offsetTable[] = {
		{+kWidth / 2.0f, -kHeight / 2.0f, 0},
		{-kWidth / 2.0f, -kHeight / 2.0f, 0},
		{+kWidth / 2.0f, +kHeight / 2.0f, 0},
		{-kWidth / 2.0f, +kHeight / 2.0f, 0}
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

Vector3 Player::GetWorldPosition() const{
	return {
		worldTransform_.matWorld_.m[3][0],
		worldTransform_.matWorld_.m[3][1],
		worldTransform_.matWorld_.m[3][2]
	};
}

AABB Player::GetAABB(){
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}

// Player.cpp の一番下に追加

// =================================================================
// 以下の関数が抜けていたので追加してください
// =================================================================

// 壁張り付き・接地状態の更新
void Player::UpdateOnGround(const CollisionMapInfo& info){
	if(onGround_){
		if(velocity_.y > 0.0f){
			onGround_ = false;
		} else{
			std::array<Vector3,kNumCorner> positionsNew;
			for(uint32_t i = 0; i < positionsNew.size(); ++i){
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
			}
			bool hit = false;
			if(mapChipField_->GetMapChipTypeByIndex(
				mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0,-kGroundSearchHeight,0)).xIndex,
				mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0,-kGroundSearchHeight,0)).yIndex) == MapChipType::kBlock){
				hit = true;
			}
			if(mapChipField_->GetMapChipTypeByIndex(
				mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0,-kGroundSearchHeight,0)).xIndex,
				mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0,-kGroundSearchHeight,0)).yIndex) == MapChipType::kBlock){
				hit = true;
			}

			if(!hit){ onGround_ = false; }
		}
	} else{
		if(info.landing){
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
	}
}

void Player::UpdateOnWall(const CollisionMapInfo& info){
	if(info.hitWall){
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}
