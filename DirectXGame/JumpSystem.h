#pragma once
#include "JumpParticle.h"
#include "ParticleManager.h"
#include <list>

class JumpSystem : public EffectSystemBase {
public:
  // プレイヤーモデルとテクスチャを受け取る
  void Initialize(Model *model, uint32_t textureHandle);

  void Update(float deltaTime) override;
  void Draw(Camera *camera) override;

  // ジャンプした位置（足元）を指定して発生させる
  void Spawn(Vector3 position);

private:
  std::list<JumpParticle> particles_;

  WorldTransform worldTransform_;
  Model *model_ = nullptr;
  uint32_t textureHandle_ = 0;
  ObjectColor colorHelper_;
};