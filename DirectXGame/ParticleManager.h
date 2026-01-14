#pragma once
#include "KamataEngine.h" // 基盤となるKamataEngineのヘッダーファイル
#include <map>            // エフェクトシステムを名前で管理するためのマップ
#include <string>         // システム名に使用する文字列
#include <vector> // 将来的な拡張用のベクトル（現在は未使用だが、インクルードされている）

using namespace KamataEngine; // KamataEngineの名前空間を使用

/// @brief すべてのエフェクトシステムが継承すべき抽象基底クラス
/// エフェクトシステムに必要な共通インターフェース（UpdateとDraw）を定義する
class EffectSystemBase {
public:
  /// @brief デストラクタ
  /// 継承クラスのデストラクタが正しく呼び出されるように virtual に設定
  virtual ~EffectSystemBase() = default;

  /// @brief 状態更新処理（純粋仮想関数 = 継承クラスでの実装が必須）
  /// @param deltaTime 前回の更新からの経過時間
  virtual void Update(float deltaTime) = 0;

  /// @brief 描画処理（純粋仮想関数 = 継承クラスでの実装が必須）
  /// @param camera 描画に使用するカメラ情報
  virtual void Draw(Camera *camera) = 0;
};

/// @brief すべてのエフェクトシステムを管理するシングルトンクラス
class ParticleManager {
private:
  // シングルトンインスタンスへの静的ポインタ
  static ParticleManager *instance;

  // エフェクトシステムの名前とインスタンスを保持するマップ
  std::map<std::string, EffectSystemBase *> systems;

  // 外部からのインスタンス生成を防ぐためのプライベートコンストラクタ
  ParticleManager() = default;

public:
  /// @brief ParticleManagerの唯一のインスタンスを取得する
  /// @return ParticleManagerのインスタンスへのポインタ
  static ParticleManager *GetInstance();

  /// @brief ParticleManagerと管理下のエフェクトシステムを初期化する
  void Initialize();

  /// @brief 管理下にあるすべてのエフェクトシステムの Update を呼び出す
  /// @param deltaTime 前回の更新からの経過時間
  void Update(float deltaTime);

  /// @brief 管理下にあるすべてのエフェクトシステムの Draw を呼び出す
  /// @param camera 描画に使用するカメラ情報
  void Draw(Camera *camera);

  /// @brief
  /// ParticleManagerと管理下のエフェクトシステムを終了・クリーンアップする
  void Shutdown();

  /// @brief 指定された名前のエフェクトシステムを取得するテンプレート関数
  /// @tparam T 取得したいエフェクトシステムの型（例: TracerSystem）
  /// @param name 登録されているシステム名
  /// @return 指定された型Tのシステムインスタンス、または見つからなければ
  /// nullptr
  template <typename T> T *GetSystem(const std::string &name) {
    // マップ内にシステム名が存在するかチェック
    if (systems.count(name)) {
      // systems.at(name) が EffectSystemBase* であるため、T*
      // にダウンキャストを試みる
      return dynamic_cast<T *>(systems.at(name));
    }
    return nullptr;
  }

  // TracerSystem専用のゲッター関数（前方宣言が必要なため、ヘッダーに記載されている可能性あり）
  class JumpSystem *GetJumpSystem();
 class BossEffectSystem* GetBossEffectSystem();
};