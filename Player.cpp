#define NOMINMAX

#include "Player.h"
#include "MapChipField.h"
#include "Math.h"

#include <algorithm>
#include <cassert>
#include <numbers>

void Player::Update() {

	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;

		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}

	WorldTransformUpdate(worldTransform_);
	WorldTransformUpdate(worldTransformAttack_);
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorRootUpdate() {

	InputMove();

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;

	if (collisionMapInfo.ceiling) {
		velocity_.y = 0;
	}

	UpdateOnWall(collisionMapInfo);

	UpdateOnGround(collisionMapInfo);

	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);

		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		behaviorRequest_ = Behavior::kAttack;
	}
}

void Player::BehaviorAttackInitialize() {

	attackParameter_ = 0;

	velocity_ = {};

	attackPhase_ = AttackPhase::kAnticipation;
}

void Player::BehaviorAttackUpdate() {

	const Vector3 attackVelocity = {0.8f, 0.0f, 0.0f};

	Vector3 velocity{};

	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kAnticipation: // 溜め動作

	default: {
		velocity = {};
		float t = static_cast<float>(attackParameter_) / kAnticipationTime;
		worldTransform_.scale_.z = EaseOut(1.0f, 0.3f, t);
		worldTransform_.scale_.y = EaseOut(1.0f, 1.6f, t);

		// 前進動作へ移行
		if (attackParameter_ >= kAnticipationTime) {
			attackPhase_ = AttackPhase::kAction;
			attackParameter_ = 0; // カウンターをリセット
		}
		break;
	}

	case AttackPhase::kAction: { // 突進動作
		if (lrDirection_ == LRDirection::kRight) {
			velocity = +attackVelocity;
		} else {
			velocity = -attackVelocity;
		}

		float t = static_cast<float>(attackParameter_) / kActionTime;
		worldTransform_.scale_.z = EaseOut(0.3f, 1.3f, t);
		worldTransform_.scale_.y = EaseIn(1.6f, 0.7f, t);

		// 余韻動作へ移行
		if (attackParameter_ >= kActionTime) {
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0; // パラメータをリセット
		}
	} break;

	case AttackPhase::kRecovery: { // 余韻動作
		velocity = {};
		float t = static_cast<float>(attackParameter_) / kRecoveryTime;
		worldTransform_.scale_.z = EaseOut(1.3f, 1.0f, t);
		worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

		// 通常行動に戻る
		if (attackParameter_ >= kRecoveryTime) {
			behaviorRequest_ = Behavior::kRoot;
		}
		break;
	}
	}

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;

	if (turnTimer_ > 0.0f) {
		// タイマーを進める
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);

		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
}

void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position) {

	assert(model);
	// モデル
	model_ = model;
	modelAttack_ = modelAttack;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	camera_ = camera;
}

void Player::InputMove() {

	if (onGround_) {

		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

				if (velocity_.x < 0.0f) {
					// 旋回の最初は移動減衰をかける
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f) {
					// 旋回の最初は移動減衰をかける
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ += acceleration;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ほぼ0の場合に0にする
		if (std::abs(velocity_.x) <= 0.0001f) {
			velocity_.x = 0.0f;
		}

		isHovering_ = false;

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ += Vector3(0, kJumpAcceleration / 60.0f, 0);
		}
	} else {
		// ホバリングの処理

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			if (!isHovering_) {
				isHovering_ = true;

				velocity_.y = std::clamp(velocity_.y, -kLimitFallSpeed, kLimitHoverRiseSpeed);
			}

			velocity_ += Vector3(0, kHoverAcceleration / 60.0f, 0);
			velocity_.y = std::clamp(velocity_.y, -kLimitHoverFallSpeed, kLimitHoverRiseSpeed);

		} else {
			isHovering_ = false;

			// 通常の落下速度
			velocity_ += Vector3(0, -kGravityAcceleration / 60.0f, 0);
			velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
		}
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			velocity_.x += kAirControlAcceleration / 60.0f;
		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			velocity_.x -= kAirControlAcceleration / 60.0f;
		}

		// 空中での左右速度の制限
		velocity_.x = std::clamp(velocity_.x, -kLimitAirSpeed, kLimitAirSpeed);
		// 非入力時の空中での左右減衰（あってもなくても良い）
		velocity_.x *= (1.0f - kAirAttenuation);
	}
}

void Player::CheckMapCollision(CollisionMapInfo& info) {

	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info) {

	if (info.move.y <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			info.ceiling = true;
		}
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {

	if (info.move.y >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	// フラグ初期化
	bool hit = false;

	// 左下の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		// 地面に当たったことを記録する
		info.landing = true;
	}
}

void Player::UpdateOnGround(const CollisionMapInfo& info) {

	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}

			bool hit = false;

			MapChipType mapChipType;

			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) {
				onGround_ = false;
			}
		}
	} else {
		if (info.landing) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
	}
}

void Player::UpdateOnWall(const CollisionMapInfo& info) {

	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info) {

	if (info.move.x <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 右側の当たり判定
	bool hit = false;

	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(+kWidth / 2.0f, 0, 0));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {

	if (info.move.x >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 右側の当たり判定
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::max(0.0f, rect.right - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, //  kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  //  kLeftTop
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Draw() {

	model_->Draw(worldTransform_, *camera_);
	if (behavior_ == Behavior::kAttack) {
		switch (attackPhase_) {
		case AttackPhase::kAnticipation:
		default:
			// 予備動作中は攻撃モデルを描画しない
			break;
		case AttackPhase::kAction:
		case AttackPhase::kRecovery:
			modelAttack_->Draw(worldTransformAttack_, *camera_);
			break;
		}
	}
}

Vector3 Player::GetWorldPosition() const {

	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Player::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {

	if (IsAttack()) {
		return; // 攻撃中はダメージ無効
	}

	// 不使用
	(void)enemy;

	isDead_ = true;

	isCollisionDisabled_ = true; // 衝突無効化
}