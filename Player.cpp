#include "Player.h"

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	camera_ = camera;
	model_ = model;
}
void Player::Update() {

	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		Vector3 acceleration = {};

		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = 0.0f;
			}

			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x += kAcceleration;
		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = 0.0f;
			}

			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x -= kAcceleration;
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}

		velocity_ += acceleration;
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		if (turnTimer_ > 0.0f) {
			turnTimer_ += 1.0f / 60.0f;

			float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
			float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
			worldTransform_.rotation_.y = EaseInOutLerp(turnFirstRotationY_, destinationRotationY, turnTimer_ / lTimeTurn);
		}
	}

	worldTransform_.translation_ += velocity_;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}
void Player::Draw() { model_->Draw(worldTransform_, *camera_); }
