#include "JumpSystem.h"

void JumpSystem::Initialize(Model *model, uint32_t textureHandle) {
  model_ = model;
  textureHandle_ = textureHandle;
  worldTransform_.Initialize();
  colorHelper_.Initialize();
}

void JumpSystem::Spawn(Vector3 position) {
  // 1回のジャンプで10個くらいの埃を出す
  int numParticles = 10;

  for (int i = 0; i < numParticles; ++i) {
    // 中心から少しずらす
    Vector3 startPos = position;
    startPos.x += (float)(rand() % 100 - 50) / 100.0f; // -0.5 ~ 0.5
    startPos.z += (float)(rand() % 100 - 50) / 100.0f;

    // 外側に広がる速度を作る
    // Y軸（高さ）は少しだけ上(0.0~0.5)に向けて、基本は横に広げる
    Vector3 velocity;
    velocity.x = (float)(rand() % 200 - 100) / 20.0f; // -5.0 ~ 5.0
    velocity.y = (float)(rand() % 50) / 20.0f;        //  0.0 ~ 2.5 (少し浮く)
    velocity.z = (float)(rand() % 200 - 100) / 20.0f; // -5.0 ~ 5.0

    particles_.emplace_back(startPos, velocity);
  }
}

void JumpSystem::Update(float deltaTime) {
  for (auto it = particles_.begin(); it != particles_.end();) {
    it->Update(deltaTime);
    if (it->IsDead()) {
      it = particles_.erase(it);
    } else {
      ++it;
    }
  }
}

void JumpSystem::Draw(Camera *camera) {
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();
  if (!model_)
    return;
  Model::PreDraw(dxCommon->GetCommandList());

  for (const auto &p : particles_) {
    worldTransform_.translation_ = p.position;
    worldTransform_.scale_ = {p.scale, p.scale, p.scale};

    // 埃っぽい色 (白〜薄いグレー)
    float alpha = p.GetAlpha();
    colorHelper_.SetColor({0.9f, 0.9f, 0.9f, alpha});

    WorldTransformUpdate(worldTransform_);
    model_->Draw(worldTransform_, *camera, textureHandle_, &colorHelper_);
  }

  Model::PostDraw();
}