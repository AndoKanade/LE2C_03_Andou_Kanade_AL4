#include "Enemy.h"
#include "GameScene.h"
#include "Math.h"
#include "Player.h"
#include <cassert>
#include <numbers>

void Enemy::Initialize(Model* model,Camera* camera,const Vector3& position,Type type){
	assert(model);
	model_ = model;
	camera_ = camera;

	// タイプを保存
	type_ = type;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> *3.0f / 2.0f;

	fireTimer_ = 120 + (rand() % 180);

	// ★追加: タイプごとの設定
	objectColor_.Initialize(); // 色用

	if(type_ == Type::kScarecrow){
		// --- カカシの設定 ---
		hp_ = 9999; // 死なないように
		velocity_ = {0, 0, 0}; // 動かない
		objectColor_.SetColor({0.5f, 1.0f, 0.5f, 1.0f}); // 緑色
		worldTransform_.scale_ = {1.5f, 1.5f, 1.5f}; // 普通サイズ
	} // Enemy.cpp Initialize内 (ボスの設定部分)

	else if(type_ == Type::kBoss){
		hp_ = 10;
		objectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f}); // 赤色で威圧感

		// Blenderで作ったモデルサイズに合わせて倍率を調整
		// もしBlenderですでに大きく作っていたら 1.0f でもOK
		worldTransform_.scale_ = {2.0f, 2.0f, 2.0f};

		// ボスはゆっくり回転させると巨大感が出る
		velocity_ = {-0.01f, 0, 0}; // 移動はゆっくり
	}

	walkTimer = 0.0f;
}

// 02_09 スライド5枚目
void Enemy::Update(){

	// 変更リクエストがあったら
	if(behaviorRequest_ != Behavior::kUnknown){
		// 振るまいを変更する
		behavior_ = behaviorRequest_;

		// 各振るまいごとの初期化を実行
		switch(behavior_){
		case Behavior::kDefeated:
		default:
			counter_ = 0;
			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	// 02_15 13枚目
	switch(behavior_){
		// 歩行
	case Behavior::kWalk:
		// 02_09 16枚目 移動
		//worldTransform_.translation_ += velocity_;

		//// 02_09 20枚目
		//walkTimer += 1.0f / 60.0f;

		//// 02_09 23枚目 回転アニメーション
		//worldTransform_.rotation_.x = std::sin(std::numbers::pi_v<float> *2.0f * walkTimer / kWalkMotionTime);

		// 02_09 スライド8枚目 ワールド行列更新
		WorldTransformUpdate(worldTransform_);
		break;
		// やられ
	case Behavior::kDefeated:
		// 02_15 15枚目
		counter_ += 1.0f / 60.0f;

		worldTransform_.rotation_.y += 0.3f;
		worldTransform_.rotation_.x = EaseOut(ToRadians(kDefeatedMotionAngleStart),ToRadians(kDefeatedMotionAngleEnd),counter_ / kDefeatedTime);

		WorldTransformUpdate(worldTransform_);

		if(counter_ >= kDefeatedTime){
			isDead_ = true;
		}
		break;
	}
}

void Enemy::Draw(){
	// ★変更: 色を渡して描画
	model_->Draw(worldTransform_,*camera_,&objectColor_);
}

void Enemy::OnCollision(const Player* player){
	// 既に死んでるなら何もしない
	if(behavior_ == Behavior::kDefeated){
		return;
	}

	(void)player; // 警告消し

	hp_ = 0; // HPを0にする
	behaviorRequest_ = Behavior::kDefeated; // 即座に死亡状態へ遷移

	// 当たり判定を無効化（死体に当たらないようにする）
	isCollisionDisabled_ = true;
}

// 02_10 スライド14枚目
AABB Enemy::GetAABB(){

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

// 02_10 スライド14枚目
Vector3 Enemy::GetWorldPosition(){

	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

bool Enemy::IsTimeToFire() {
	// 既に死んでいたら撃たない
	if (behavior_ == Behavior::kDefeated) {
		return false;
	}

	// タイマーを減らす
	fireTimer_--;

	// タイマーが0になったら発射！
	if (fireTimer_ <= 0) {
		// 次の発射までの時間をセット (ここも少しランダムにするとさらに良い)
		// 例: 120フレーム(2秒) + ランダム60フレーム(1秒)
		fireTimer_ = 120 + (rand() % 60);
		
		return true; // 「撃ってよし！」と返す
	}

	return false; // まだ待ち
}