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
	// ジャンプエフェクトの準備
	JumpSystem* jump = ParticleManager::GetInstance()->GetJumpSystem();
	if(jump){
		jump->Initialize(model_,textureHandle_);
	}

	// --- パラメータ初期化 ---
	// デフォルトの能力を設定
	abilityType_ = AbilityType::Sword;
	objectColor_.Initialize();
}

// =================================================================
// 更新処理 (メインループ)
// =================================================================
void Player::Update(){

	// --- 行動遷移リクエストの処理 ---
	// 振る舞い（Behavior）の変更要求があれば切り替える
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
		BehaviorAttackUpdate(); // ソード攻撃
		break;
	case Behavior::kShot:
		BehaviorShotUpdate(); // ビーム射撃（硬直）
		break;
	}

	// --- 色の反映 ---
	// 能力に応じてプレイヤーの色を変える
	if(abilityType_ == AbilityType::Sword){
		objectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f}); // ソード：赤
	} else if(abilityType_ == AbilityType::Beam){
		objectColor_.SetColor({1.0f, 1.0f, 0.0f, 1.0f}); // ビーム：黄色
	}

	// --- 行列更新 ---
	WorldTransformUpdate(worldTransform_);
	WorldTransformUpdate(worldTransformAttack_);
}

// =================================================================
// 描画処理
// =================================================================
void Player::Draw(){

	// 1. プレイヤー本体の描画
	model_->Draw(worldTransform_,*camera_,&objectColor_);

	// 2. 攻撃モデル（剣）の描画
	// ソード攻撃中 (kAttack) かつ、アクション中のみ表示する制御
	if(behavior_ == Behavior::kAttack){
		switch(attackPhase_){
		case AttackPhase::kAnticipation:
			// 予備動作（溜め）中は表示しない
			break;
		case AttackPhase::kAction:
		case AttackPhase::kRecovery:
			// 必要であればコメントアウトを解除して描画
			// modelAttack_->Draw(worldTransformAttack_, *camera_);
			break;
		default:
			break;
		}
	}
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

	// 移動反映
	worldTransform_.translation_ += collisionMapInfo.move;

	// 天井・壁・接地処理
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

	// --- 能力切り替えと攻撃 ---

	// [1]キー: ソードモード
	if(Input::GetInstance()->TriggerKey(DIK_1)){
		abilityType_ = AbilityType::Sword;
	}
	// [2]キー: ビームモード
	if(Input::GetInstance()->TriggerKey(DIK_2)){
		abilityType_ = AbilityType::Beam;
	}

	// [SPACE]キー: 攻撃
	if(Input::GetInstance()->TriggerKey(DIK_SPACE)){
		if(abilityType_ == AbilityType::Sword){
			behaviorRequest_ = Behavior::kAttack; // 近接攻撃へ
		} else if(abilityType_ == AbilityType::Beam){
			behaviorRequest_ = Behavior::kShot;   // 射撃へ
		}
	}
}

// =================================================================
// 振る舞い: ソード攻撃 (Attack)
// =================================================================
void Player::BehaviorAttackInitialize(){
	attackParameter_ = 0;
	velocity_ = {};
	attackPhase_ = AttackPhase::kAnticipation;
}

void Player::BehaviorAttackUpdate(){
	const Vector3 attackVelocity = {0.8f, 0.0f, 0.0f};
	Vector3 velocity{};
	attackParameter_++;

	switch(attackPhase_){
	case AttackPhase::kAnticipation: // 溜め
	default:
	{
		velocity = {};
		float t = static_cast<float>(attackParameter_) / kAnticipationTime;
		// 縮んで溜める演出
		worldTransform_.scale_.z = EaseOut(1.0f,0.3f,t);
		worldTransform_.scale_.y = EaseOut(1.0f,1.6f,t);

		if(attackParameter_ >= kAnticipationTime){
			attackPhase_ = AttackPhase::kAction;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kAction:
	{ // 突進
		if(lrDirection_ == LRDirection::kRight){
			velocity = +attackVelocity;
		} else{
			velocity = -attackVelocity;
		}
		float t = static_cast<float>(attackParameter_) / kActionTime;
		// 伸びて突く演出
		worldTransform_.scale_.z = EaseOut(0.3f,1.3f,t);
		worldTransform_.scale_.y = EaseIn(1.6f,0.7f,t);

		if(attackParameter_ >= kActionTime){
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kRecovery:
	{ // 硬直(戻り)
		velocity = {};
		float t = static_cast<float>(attackParameter_) / kRecoveryTime;
		// 元の形に戻る
		worldTransform_.scale_.z = EaseOut(1.3f,1.0f,t);
		worldTransform_.scale_.y = EaseOut(0.7f,1.0f,t);

		if(attackParameter_ >= kRecoveryTime){
			behaviorRequest_ = Behavior::kRoot;
		}
		break;
	}
	}

	// 攻撃中の移動処理（壁抜け防止）
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity;
	CheckMapCollision(collisionMapInfo);
	worldTransform_.translation_ += collisionMapInfo.move;

	// 攻撃モデルの位置合わせ
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
}

// =================================================================
// 振る舞い: ビーム射撃 (Shot)
// =================================================================
void Player::BehaviorShotInitialize(){
	// 足を止める（慣性を消す）
	velocity_ = {};

	// 硬直タイマーをセット
	shotTimer_ = kShotTime;

	// GameScene側で検知させるためのフラグを立てる
	isShotBeamRequest_ = true;
}

void Player::BehaviorShotUpdate(){
	// 重力処理（空中で撃った時に不自然に止まらないように）
	velocity_.y += -kGravityAcceleration / 60.0f;
	velocity_.y = std::max(velocity_.y,-kLimitFallSpeed);

	// マップ衝突判定
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;
	UpdateOnGround(collisionMapInfo);

	// タイマー更新
	shotTimer_--;

	// 硬直終了で通常状態へ戻る
	if(shotTimer_ <= 0){
		behaviorRequest_ = Behavior::kRoot;
	}
}

// =================================================================
// 入力と物理挙動 (InputMove)
// =================================================================
void Player::InputMove(){

	if(onGround_){
		// --- 地上移動 ---
		isHovering_ = false;

		// 左右移動
		if(Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)){
			Vector3 acceleration = {};

			if(Input::GetInstance()->PushKey(DIK_RIGHT)){
				// 切り返し時の減衰
				if(velocity_.x < 0.0f){ velocity_.x *= (1.0f - kAttenuation); }

				acceleration.x += kAcceleration / 60.0f;

				// 向き変更
				if(lrDirection_ != LRDirection::kRight){
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if(Input::GetInstance()->PushKey(DIK_LEFT)){
				// 切り返し時の減衰
				if(velocity_.x > 0.0f){ velocity_.x *= (1.0f - kAttenuation); }

				acceleration.x -= kAcceleration / 60.0f;

				// 向き変更
				if(lrDirection_ != LRDirection::kLeft){
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ += acceleration;
			velocity_.x = std::clamp(velocity_.x,-kLimitRunSpeed,kLimitRunSpeed);
		} else{
			// 摩擦で減速
			velocity_.x *= (1.0f - kAttenuation);
		}

		// 微小な速度は0にする
		if(std::abs(velocity_.x) <= 0.0001f){
			velocity_.x = 0.0f;
		}

		// --- ジャンプ ---
		if(Input::GetInstance()->PushKey(DIK_UP)){
			velocity_ += Vector3(0,kJumpAcceleration / 60.0f,0);

			// ジャンプエフェクト発生
			JumpSystem* jumpSys = ParticleManager::GetInstance()->GetJumpSystem();
			if(jumpSys){
				Vector3 footPos = worldTransform_.translation_;
				footPos.y -= 1.0f; // 足元調整
				jumpSys->Spawn(footPos);
			}
		}
	} else{
		// --- 空中挙動 (ホバリング等) ---
		isHovering_ = true;

		if(Input::GetInstance()->TriggerKey(DIK_UP)){
			// ホバリング上昇
			velocity_.y += kHoverImpulse;
			velocity_.y = std::min(velocity_.y,kLimitHoverImpulseSpeed);
		} else{
			// 通常落下
			isHovering_ = false;
			velocity_ += Vector3(0,-kGravityAcceleration / 60.0f,0);
			velocity_.y = std::max(velocity_.y,-kLimitFallSpeed);
		}

		// 空中制御
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
// 衝突判定系 (Collision)
// =================================================================

// 敵との衝突 (ダメージや無敵判定など)
void Player::OnCollision(const Enemy* enemy){
	// 攻撃中は無敵
	if(IsAttack()){
		return;
	}

	(void)enemy; // 未使用変数の警告消し

	// テスト用ロジック（必要に応じて有効化してください）
	canICrear = true;
	counter++;
	if(canICrear){
		if(counter <= 3000){
			canICrear1 = true;
		}
	}

	// isDead_ = true;          // 死亡フラグ
	isCollisionDisabled_ = true; // 一時的に衝突無効化
}

// -------------------------------------------------------------
// マップチップ衝突判定 (詳細実装)
// -------------------------------------------------------------
void Player::CheckMapCollision(CollisionMapInfo& info){
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

// 上方向の判定
void Player::CheckMapCollisionUp(CollisionMapInfo& info){
	if(info.move.y <= 0) return;

	std::array<Vector3,kNumCorner> positionsNew;
	for(uint32_t i = 0; i < positionsNew.size(); ++i){
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
	}

	bool hit = false;
	// 左上
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]).yIndex) == MapChipType::kBlock){
		hit = true;
	}
	// 右上
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

// 下方向の判定 (接地)
void Player::CheckMapCollisionDown(CollisionMapInfo& info){
	if(info.move.y >= 0) return;

	std::array<Vector3,kNumCorner> positionsNew;
	for(uint32_t i = 0; i < positionsNew.size(); ++i){
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move,static_cast<Corner>(i));
	}

	bool hit = false;
	// 左下
	if(mapChipField_->GetMapChipTypeByIndex(
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]).xIndex,
		mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]).yIndex) == MapChipType::kBlock){
		hit = true;
	}
	// 右下
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

// 右方向の判定
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

// 左方向の判定
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

// -------------------------------------------------------------
// ユーティリティ
// -------------------------------------------------------------
Vector3 Player::CornerPosition(const Vector3& center,Corner corner){
	Vector3 offsetTable[] = {
		{+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
		{-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
		{+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
		{-kWidth / 2.0f, +kHeight / 2.0f, 0}  // kLeftTop
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