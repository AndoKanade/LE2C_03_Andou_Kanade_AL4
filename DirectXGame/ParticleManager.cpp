#include "ParticleManager.h"
#include "JumpSystem.h"
#include "BossEffectSystem.h"
#include <iostream>

// -----------------------------------------------------------------
// シングルトンと初期化
// -----------------------------------------------------------------

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance(){
	if(!instance){
		instance = new ParticleManager();
	}
	return instance;
}

void ParticleManager::Initialize(){
	std::cout << "ParticleManager: Initialization started." << std::endl;

	// ★修正箇所: 
	// 1. 未使用変数の削除 (JumpSystem* jumpSystem = ... を削除)
	// 2. キー名を "JumpSystem" に統一 (GetJumpSystemでこの名前で探しているため)
	systems["JumpSystem"] = new JumpSystem();
	systems["BossEffectSystem"] = new BossEffectSystem();

	std::cout << "ParticleManager: Initialization complete. Systems registered: "
		<< systems.size() << std::endl;
}

// -----------------------------------------------------------------
// 更新と描画
// -----------------------------------------------------------------

void ParticleManager::Update(float deltaTime){
	for(auto const& [name,system] : systems){
		system->Update(deltaTime);
	}
}

void ParticleManager::Draw(Camera* camera){
	for(auto const& [name,system] : systems){
		system->Draw(camera);
	}
}

// -----------------------------------------------------------------
// 終了処理とアクセス関数
// -----------------------------------------------------------------

void ParticleManager::Shutdown(){
	std::cout << "ParticleManager: Shutting down and cleaning up resources."
		<< std::endl;

	for(auto const& [name,system] : systems){
		delete system;
	}
	systems.clear();

	delete instance;
	instance = nullptr;
}

JumpSystem* ParticleManager::GetJumpSystem(){
	// Initializeで登録した名前("JumpSystem")と同じキーで取得する
	return GetSystem<JumpSystem>("JumpSystem");
}

BossEffectSystem* ParticleManager::GetBossEffectSystem(){
	return GetSystem<BossEffectSystem>("BossEffectSystem");
}