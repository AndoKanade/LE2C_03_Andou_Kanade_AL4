#include "WallHitEffectSystem.h"
#include"Math.h"

void WallHitEffectSystem::Initialize(Model* model){
	model_ = model;
}

void WallHitEffectSystem::Update(float deltaTime){
	// deltaTimeは今回使わなくてもOKですが、引数は合わせる必要があります
	(void)deltaTime;

	for(auto it = particles_.begin(); it != particles_.end(); ){
		WallHitParticle* p = *it;

		// 移動
		p->worldTransform.translation_ += p->velocity;
		p->velocity.y -= 0.02f; // 重力

		// 行列更新
		WorldTransformUpdate(p->worldTransform);

		// 寿命を減らす
		p->lifeTimer--;

		// 徐々に透明にする
		float alpha = (float)p->lifeTimer / 20.0f; // 20フレームで消える
		p->objectColor.SetColor({1.0f, 0.8f, 0.0f, alpha}); // 黄色っぽい色

		// 寿命切れで削除
		if(p->lifeTimer <= 0){
			delete p;
			it = particles_.erase(it);
		} else{
			++it;
		}
	}
}

void WallHitEffectSystem::Draw(Camera* camera){
	// シンプルに描画のみ行う
	for(WallHitParticle* p : particles_){
		model_->Draw(p->worldTransform,*camera,&p->objectColor);
	}
}

void WallHitEffectSystem::Spawn(const Vector3& position){
	// 1回のヒットで5粒くらい出す
	for(int i = 0; i < 5; i++){
		WallHitParticle* p = new WallHitParticle();
		p->worldTransform.Initialize();
		p->objectColor.Initialize();

		p->worldTransform.translation_ = position;
		// サイズは小さく
		p->worldTransform.scale_ = {0.15f, 0.15f, 0.15f};
		// 色は初期値（黄色）
		p->objectColor.SetColor({1.0f, 0.8f, 0.0f, 1.0f});
		p->lifeTimer = 20; // 寿命は短く(約0.3秒)

		// ランダムな方向に飛び散らせる
		Vector3 velocity;
		velocity.x = (rand() % 100 - 50) / 100.0f;
		velocity.y = (rand() % 100 - 50) / 100.0f;
		velocity.z = (rand() % 100 - 50) / 100.0f;
		// 速度調整
		p->velocity = {velocity.x * 0.2f, velocity.y * 0.2f, velocity.z * 0.2f};

		particles_.push_back(p);
	}
}