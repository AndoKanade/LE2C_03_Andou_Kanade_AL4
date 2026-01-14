#include "Beam.h"

// ==========================================
// Beamクラスの実装
// ==========================================

void Beam::Initialize(Model* model,const Vector3& position,const Vector3& velocity){
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f}; // サイズ調整
	velocity_ = velocity;

	objectColor_.Initialize();
	objectColor_.SetColor({1.0f, 1.0f, 0.0f, 1.0f}); // 黄色	
}

void Beam::Update(){
	// 移動
	worldTransform_.translation_ += velocity_;

	// 行列更新
	WorldTransformUpdate(worldTransform_);

	// 寿命処理
	if(--lifeTimer_ <= 0){
		isDead_ = true;
	}
}

void Beam::Draw(const Camera& camera){
	model_->Draw(worldTransform_,camera,&objectColor_);
}

void Beam::OnCollision(){
	isDead_ = true;
}
