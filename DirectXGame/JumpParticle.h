#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

class JumpParticle {
public:
  Vector3 position;
  Vector3 velocity; // 飛び散る方向と速さ
  float scale;      // 大きさ

  float maxTime;
  float currentTime;

  // コンストラクタ
  JumpParticle(Vector3 pos, Vector3 vel) : position(pos), velocity(vel) {
    scale = 0.8f;   // 最初はちょっと大きめ
    maxTime = 0.5f; // 0.5秒で消える
    currentTime = maxTime;
  }

  void Update(float deltaTime) {
    currentTime -= deltaTime;

    // 移動
    position += velocity * deltaTime;

    // 徐々に小さくする
    scale -= 1.0f * deltaTime;
    if (scale < 0.0f)
      scale = 0.0f;
  }

  bool IsDead() const { return currentTime <= 0.0f || scale <= 0.0f; }

  float GetAlpha() const { return currentTime / maxTime; }
};