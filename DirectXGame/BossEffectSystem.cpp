#include "BossEffectSystem.h"
#include <cstdlib> // rand()用

using namespace KamataEngine;

void BossEffectSystem::Initialize(Model* model){
	model_ = model;

	// 全て非アクティブにしておく
	for(auto& p : particles_){
		p.isActive = false;
		p.color.Initialize();
		p.worldTransform.Initialize();

	}
	nextIndex_ = 0;
}

void BossEffectSystem::Spawn(const Vector3& centerPos){
	// ★検索せず、強制的に「次の場所」を使う（古いものは上書きされる）
	BossParticle& p = particles_[nextIndex_];

	p.isActive = true;

	// (座標計算などはそのまま...)
	float offsetX = (rand() % 300 - 150) / 100.0f;
	float offsetY = (rand() % 300 - 150) / 100.0f;
	float offsetZ = (rand() % 300 - 150) / 100.0f;

	p.position = {centerPos.x + offsetX, centerPos.y + offsetY, centerPos.z + offsetZ};
	p.velocity = {0.0f, 0.05f, 0.0f};
	p.scale = 0.1f;
	p.maxLife = 60.0f;
	p.lifeTimer = p.maxLife;
	p.color.SetColor({1.0f, 0.8f, 0.2f, 1.0f});

	// ★インデックスを進める（最後まで行ったら0に戻る）
	nextIndex_ = (nextIndex_ + 1) % kMaxParticles;
}

void BossEffectSystem::Update(float deltaTime){
	(void)deltaTime;

	for(auto& p : particles_){
		if(p.isActive){
			// 移動計算
			p.position.x += p.velocity.x;
			p.position.y += p.velocity.y;
			p.position.z += p.velocity.z;

			// ... (寿命計算などはそのまま) ...

			// ★追加: 計算した座標とサイズを worldTransform に反映させる
			p.worldTransform.translation_ = p.position;
			p.worldTransform.scale_ = {p.scale, p.scale, p.scale};

			// 行列の更新 (UpdateMatrix ではなく WorldTransformUpdate を使う)
			WorldTransformUpdate(p.worldTransform);
		}
	}
}
void BossEffectSystem::Draw(Camera* camera){
	if(!model_) return;

	for(auto& p : particles_){
		if(p.isActive){

			model_->Draw(p.worldTransform,*camera,&p.color);
		}
	}
}