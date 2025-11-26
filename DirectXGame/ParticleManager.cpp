#include "ParticleManager.h" // 宣言が書かれているヘッダー
#include "JumpSystem.h"
#include <iostream> // デバッグログ出力用

// -----------------------------------------------------------------
// シングルトンと初期化
// -----------------------------------------------------------------

// 静的メンバ変数の初期化
// ParticleManagerの唯一のインスタンスへのポインタをnullptrで初期化
ParticleManager *ParticleManager::instance = nullptr;

/// @brief ParticleManagerのインスタンスを取得する
/// 初回呼び出し時にインスタンスを生成する（遅延初期化）
/// @return ParticleManagerのインスタンスへのポインタ
ParticleManager *ParticleManager::GetInstance() {
  if (!instance) {
    // インスタンスがまだ存在しない場合、新しく生成
    instance = new ParticleManager();
  }
  return instance;
}

/// @brief 全てのエフェクトシステムを初期化し、管理リストに登録する
void ParticleManager::Initialize() {
  std::cout << "ParticleManager: Initialization started." << std::endl;

  // ... その他のシステムも同様に登録（例：systems["Explosion"] = new
  // ExplosionSystem();）
  JumpSystem *jumpSystem = new JumpSystem();
  systems["JumpDust"] = new JumpSystem();

  std::cout << "ParticleManager: Initialization complete. Systems registered: "
            << systems.size() << std::endl;
}

// -----------------------------------------------------------------
// 更新と描画 (メインループから呼ばれる処理)
// -----------------------------------------------------------------

/// @brief 全ての管理システムに対し、Update関数を呼び出す
/// @param deltaTime 前フレームからの経過時間
void ParticleManager::Update(float deltaTime) {
  // マップ内の全てのシステムをイテレート
  // C++17の構造化束縛で name (string) と system (EffectSystemBase*) を取得
  for (auto const &[name, system] : systems) {
    system->Update(deltaTime);
  }
}

/// @brief 全ての管理システムに対し、Draw関数を呼び出す
/// @param camera 描画に使用するカメラ情報
void ParticleManager::Draw(Camera *camera) {
  // マップ内の全てのシステムをイテレート
  for (auto const &[name, system] : systems) {
    system->Draw(camera);
  }
  // NOTE: 描画順序が重要であれば、ここで描画順を制御するロジックを追加する
  //       (例: マップではなく、描画順に並べた vector<EffectSystemBase*>
  //       を使用する)
}

// -----------------------------------------------------------------
// 終了処理とアクセス関数
// -----------------------------------------------------------------

/// @brief 全てのシステムとParticleManager自身のリソースを解放する
void ParticleManager::Shutdown() {
  std::cout << "ParticleManager: Shutting down and cleaning up resources."
            << std::endl;

  // 登録されたシステムを全て解放し、メモリリークを防ぐ
  for (auto const &[name, system] : systems) {
    delete system; // new で生成された TracerSystem などのオブジェクトを解放
  }
  systems.clear(); // マップの中身をクリア

  // シングルトンインスタンス自身を解放し、ポインタをリセット
  delete instance;
  instance = nullptr;
}

JumpSystem *ParticleManager::GetJumpSystem() {
  return GetSystem<JumpSystem>("JumpSystem");
}