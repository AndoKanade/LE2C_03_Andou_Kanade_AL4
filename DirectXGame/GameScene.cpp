#include "GameScene.h"
#include "Math.h"
#include "ParticleManager.h"
#include "BossEffectSystem.h"
#include "WallHitEffectSystem.h"
#include <windows.h>
#include <string>

// ==========================================
// GameSceneの実装
// ==========================================
using namespace KamataEngine;

// --- エフェクト生成 ---
void GameScene::CreateEffect(const Vector3& position){
	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}

// --- デストラクタ (終了処理) ---
GameScene::~GameScene(){

	// 2Dリソース
	delete sprite_;

	// モデル類
	delete model_;
	delete modelArrow_;
	delete modelBoss_;
	delete modelBlock_;
	delete modelSkydome_;
	delete modelPlayer_;
	delete modelAttack_;
	delete modelDeathEffect_;
	delete modelBeam_;
	// modelParticle_ はParticleManager等で共用される場合があるため注意が必要だが、
	// ここではCreateFromOBJで作ったものなら消してOK
	// delete modelParticle_; // (メンバ変数として持っているなら)

	// ステージ情報
	for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
		for(WorldTransform* worldTransformBlock : worldTransformBlockLine){
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	delete mapChipField_;

	// カメラ
	delete debugCamera_;

	// キャラクター・敵
	delete player_; // GameSceneで生成したplayerはここで消す
	for(Enemy* enemy : enemies_){
		delete enemy;
	}
	enemies_.clear();

	// エフェクト・弾
	delete deathParticles_;
	for(HitEffect* hitEffect : hitEffects_){
		delete hitEffect;
	}
	hitEffects_.clear();

	for(Beam* beam : beams_){
		delete beam;
	}
	beams_.clear();
}

// --- 初期化処理 ---
void GameScene::Initialize(){

	// カメラ設定
	camera_.Initialize();
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth,WinApp::kWindowHeight);

	// 背景（天球）
	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("SkyDome",true);
	skydome_->Initialize(&camera_,modelSkydome_);

	// マップ読み込み
	mapChipField_ = new MapChipField;
	// ★CSVから読み込み (MapChip2.csv のままでOKです)
	mapChipField_->LoadMapChipCsv("Resources/MapChip2.csv");

	modelBlock_ = Model::CreateFromOBJ("block");
	GenerateBlocks(); // ブロック配置

	// プレイヤー生成
	player_ = new Player();
	modelPlayer_ = Model::CreateFromOBJ("player");
	modelAttack_ = Model::CreateFromOBJ("attack_effect");
	// プレイヤーの初期位置も、必要ならCSVから読み取るように改造できますが、一旦このままで
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(5,18);

	player_->SetMapChipField(mapChipField_);
	player_->Initialize(modelPlayer_,modelAttack_,&camera_,playerPosition);

	// カメラコントローラー
	cameraController_ = new CameraController();
	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);

	// --- 敵の生成 ---
	modelBoss_ = Model::CreateFromOBJ("enemy");

	// ===========================================================
	// ★変更: 手動配置コードを削除し、CSVから自動生成するように変更
	// ===========================================================
	GenerateEnemies();


	// --- エフェクト・UI関連 ---
	modelDeathEffect_ = Model::CreateFromOBJ("deathParticle");
	modelParticle_ = Model::CreateFromOBJ("sphere"); // エフェクト用球体モデル
	modelBeam_ = Model::CreateFromOBJ("sphere");     // ビーム用モデル

	HitEffect::SetModel(modelParticle_);
	HitEffect::SetCamera(&camera_);

	// 矢印などのUI
	modelArrow_ = Model::CreateFromOBJ("arrow");
	textureHandle_ = TextureManager::Load("sample.png");
	sprite_ = Sprite::Create(textureHandle_,{100, 50});

	// フェード初期化
	phase_ = Phase::kFadeIn;
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn,1.0f);

	// パーティクルマネージャー初期化
	// ※ main.cppで Initialize() されている前提ならここは不要だが、
	// シーンごとにリセットするならモデルの再登録などが必要
	// ParticleManager::GetInstance()->Initialize(); // mainで呼んでいれば不要
	ParticleManager::GetInstance()->GetBossEffectSystem()->Initialize(modelParticle_);
	ParticleManager::GetInstance()->GetWallHitEffectSystem()->Initialize(modelParticle_);
}

// --- フェーズ遷移管理 ---
void GameScene::ChangePhase(){
	switch(phase_){
	case Phase::kPlay:
		if(player_->IsDead()){
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathEffect_,&camera_,deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		// 演出待ちなどはUpdate側で行う
		break;
	}
}

// --- ブロック生成 ---
void GameScene::GenerateBlocks(){
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for(uint32_t i = 0; i < numBlockVirtical; ++i){
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	for(uint32_t i = 0; i < numBlockVirtical; ++i){
		for(uint32_t j = 0; j < numBlockHorizontal; ++j){
			if(mapChipField_->GetMapChipTypeByIndex(j,i) == MapChipType::kBlock){
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j,i);
			}
		}
	}
}

// =================================================================
// 更新処理
// =================================================================
void GameScene::Update(){

	// --- 全体共有の更新 ---
	ParticleManager::GetInstance()->Update(1.0f);

	// 死亡・消滅フラグの回収 (remove_if)
	hitEffects_.remove_if([](HitEffect* hitEffect){
		if(hitEffect->IsDead()){ delete hitEffect; return true; }
		return false;
		});
	enemies_.remove_if([](Enemy* enemy){
		if(enemy->IsDead()){ delete enemy; return true; }
		return false;
		});
	beams_.remove_if([](Beam* beam){
		if(beam->IsDead()){ delete beam; return true; }
		return false;
		});

	// フェーズチェック
	ChangePhase();

	// --- フェーズごとの更新 ---
	switch(phase_){
	case Phase::kFadeIn:
		fade_->Update();
		if(fade_->IsFinished()){
			fade_->Start(Fade::Status::FadeOut,1.0f);
			phase_ = Phase::kPlay;
		}
		// フェードイン中も裏でゲームを動かす（描画のため）
		UpdateGameObjects();
		break;

	case Phase::kPlay:
		UpdateGameObjects(); // ゲーム本編の更新

		// ボスの生存チェック (クリア判定)
		{
			bool isBossAlive = false;
			for(Enemy* enemy : enemies_){
				if(enemy->GetType() == Enemy::Type::kBoss){
					isBossAlive = true;

					// ボス用エフェクトの発生 (生存中のみ)
					// ※ 頻度はランダムなどで調整しても良い
					for(int i = 0; i < 2; i++){
						Vector3 bossPos = enemy->GetWorldPosition();
						ParticleManager::GetInstance()->GetBossEffectSystem()->Spawn(bossPos);
					}
					break;
				}
			}
			// ボスがいなければクリアへ
			if(!isBossAlive){
				phase_ = Phase::kFadeOut;
			}
		}

		// 当たり判定処理
		CheckAllCollisions();
		break;

	case Phase::kDeath:
		// 死亡演出中
		if(deathParticles_ && deathParticles_->IsFinished()){
			phase_ = Phase::kFadeOut;
		}
		UpdateGameObjects(); // 背景などは動かし続ける
		if(deathParticles_){
			deathParticles_->Update();
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if(fade_->IsFinished()){
			finished_ = true; // メインループへ終了通知
		}
		UpdateGameObjects();
		break;
	}
}

// --- ゲームオブジェクトの一括更新 ---
// (各フェーズで重複していたコードをまとめました)
void GameScene::UpdateGameObjects(){
	skydome_->Update();

	// カメラ処理
	if(isDebugCameraActive_){
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else{
		cameraController_->Update();
		camera_.UpdateMatrix();
	}

	player_->Update();

	for(Enemy* enemy : enemies_){
		enemy->Update();
	}

	for(HitEffect* hitEffect : hitEffects_){
		hitEffect->Update();
	}

	// ブロック行列更新
	for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
		for(WorldTransform*& worldTransformBlock : worldTransformBlockLine){
			if(!worldTransformBlock) continue;
			WorldTransformUpdate(*worldTransformBlock);
		}
	}

	// --- ビームの発射と更新 ---
	if(player_->IsShotBeam()){
		Beam* newBeam = new Beam();
		Vector3 startPos = player_->GetWorldPosition();
		float rotY = player_->GetWorldTransform().rotation_.y;
		Vector3 velocity = {sinf(rotY), 0, cosf(rotY)};
		velocity *= 0.5f; // 弾速
		newBeam->Initialize(modelBeam_,startPos,velocity);
		beams_.push_back(newBeam);
	}

	for(Enemy* enemy : enemies_){
		// ボスタイプ、かつ、敵ごとのタイマーが0になったら
		if(enemy->GetType() == Enemy::Type::kBoss && enemy->IsTimeToFire()){

			// ★ここから下は前のコードと同じ（ビーム生成）
			Beam* newBullet = new Beam();
			Vector3 bossPos = enemy->GetWorldPosition();
			Vector3 targetPos = player_->GetWorldPosition();
			Vector3 velocity = targetPos - bossPos;

			float len = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
			if(len > 0.0f){
				velocity.x /= len;
				velocity.y /= len;
				velocity.z /= len;
			}
			Vector3 speed = {velocity.x * 0.3f, velocity.y * 0.3f, velocity.z * 0.3f};

			newBullet->Initialize(modelBeam_,bossPos,speed);
			newBullet->SetIsEnemy(true);

			beams_.push_back(newBullet);
		}
	}

	for(Beam* beam : beams_){
		beam->Update();
	}
}

// =================================================================
// 描画処理
// =================================================================
void GameScene::Draw(){

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());

	// 1. 背景・ステージ
	skydome_->Draw();
	for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
		for(WorldTransform*& worldTransformBlock : worldTransformBlockLine){
			if(!worldTransformBlock) continue;
			modelBlock_->Draw(*worldTransformBlock,camera_);
		}
	}

	// 2. キャラクター
	if(!player_->IsDead()){
		player_->Draw();
	}
	for(Enemy* enemy : enemies_){
		enemy->Draw();
	}

	// 3. エフェクト・弾
	for(Beam* beam : beams_){
		beam->Draw(camera_);
	}
	if(deathParticles_){
		deathParticles_->Draw();
	}
	for(HitEffect* hitEffect : hitEffects_){
		hitEffect->Draw();
	}

	// 4. パーティクル一括描画
	ParticleManager::GetInstance()->Draw(&camera_);

	Model::PostDraw();


	// 5. 2D UI・フェード
	Sprite::PreDraw(dxCommon->GetCommandList());
	// (もしUIがあればここで描画)
	Sprite::PostDraw();

	// フェードは一番最後（最前面）
	fade_->Draw();
}

// =================================================================
// 衝突判定
// =================================================================
void GameScene::CheckAllCollisions(){

	AABB aabb1,aabb2;

	// --- 1. 自キャラ vs 敵キャラ (体当たり) ---
	{
		aabb1 = player_->GetAABB();
		for(Enemy* enemy : enemies_){
			if(enemy->IsCollisionDisabled()) continue;

			aabb2 = enemy->GetAABB();
			if(IsCollision(aabb1,aabb2)){
				player_->OnCollision(enemy);
				enemy->OnCollision(player_);
			}
		}
	}

	for(auto it = beams_.begin(); it != beams_.end(); ){
		Beam* beam = *it;
		Vector3 bPos = beam->GetWorldPosition();
		bool hitWall = false;

		// 全ブロックと総当たりチェック
		// ブロックのリスト(worldTransformBlocks_)を使って判定します
		for(auto& line : worldTransformBlocks_){
			for(WorldTransform* block : line){
				if(!block) continue; // ブロックがない場所はスキップ

				// ブロックの範囲 (中心から半径0.5の箱とみなす)
				float minX = block->translation_.x - 0.5f;
				float maxX = block->translation_.x + 0.5f;
				float minY = block->translation_.y - 0.5f;
				float maxY = block->translation_.y + 0.5f;
				float minZ = block->translation_.z - 0.5f;
				float maxZ = block->translation_.z + 0.5f;

				// ビームがブロックの中にあるか？
				if(bPos.x >= minX && bPos.x <= maxX &&
					bPos.y >= minY && bPos.y <= maxY &&
					bPos.z >= minZ && bPos.z <= maxZ){

					hitWall = true;

					// 壁ヒットエフェクト発生（もしエラーが出るならこの行は消してください）
					ParticleManager::GetInstance()->GetWallHitEffectSystem()->Spawn(bPos);

					break;
				}
			}
			if(hitWall) break;
		}

		if(hitWall){
			delete beam;
			it = beams_.erase(it); // ビーム削除
		} else{
			++it; // 次のビームへ
		}
	}

	// --- 2. ビーム(プレイヤーの攻撃) vs 敵 ---
	for(auto itBeam = beams_.begin(); itBeam != beams_.end(); ++itBeam){
		Beam* beam = *itBeam;
		if(beam->IsEnemy()){
			continue; // 敵の弾ならスキップ（自爆防止）
		}

		for(Enemy* enemy : enemies_){
			// 簡易的な球判定（距離の2乗チェック）
			Vector3 posA = beam->GetWorldPosition();
			Vector3 posB = enemy->GetWorldPosition();
			Vector3 diff = posA - posB;
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float r = beam->GetRadius() + enemy->GetRadius();
			if(distSq <= r * r){
				// ヒット処理
				beam->OnCollision();         // ビーム消滅
				enemy->OnCollision(player_); // 敵へダメージ通知

				// ヒットエフェクト発生
				CreateEffect(posB);
				break; // 1体に当たったら消える
			}
		}
	}

	// --- 3. 吸い込み判定 (プレイヤーの口 vs 敵の弾) ---
	if(player_->IsInhaling()){

		AABB inhaleArea = player_->GetInhaleArea();

		for(auto it = beams_.begin(); it != beams_.end(); ){
			Beam* beam = *it;
			Vector3 bPos = beam->GetWorldPosition();

			// ビームが吸い込み範囲に入っているか？
			if(bPos.x >= inhaleArea.min.x && bPos.x <= inhaleArea.max.x &&
				bPos.y >= inhaleArea.min.y && bPos.y <= inhaleArea.max.y &&
				bPos.z >= inhaleArea.min.z && bPos.z <= inhaleArea.max.z){

				// ★吸い込み成功！
				delete beam;
				it = beams_.erase(it);

				player_->CatchAmmo(); // 満腹にする
				break;
			} else{
				++it;
			}
		}
	}

	// --- 4. 敵の弾 vs プレイヤー本体 (ダメージ判定) ---
	AABB playerBodyBox = player_->GetAABB();

	for(auto it = beams_.begin(); it != beams_.end(); ){
		Beam* beam = *it;

		// 1. 敵の弾じゃなければ無視
		if(!beam->IsEnemy()){
			++it;
			continue;
		}

		// 2. 当たり判定（プレイヤーの体の中に弾があるか？）
		Vector3 bPos = beam->GetWorldPosition();
		if(bPos.x >= playerBodyBox.min.x && bPos.x <= playerBodyBox.max.x &&
			bPos.y >= playerBodyBox.min.y && bPos.y <= playerBodyBox.max.y &&
			bPos.z >= playerBodyBox.min.z && bPos.z <= playerBodyBox.max.z){

			// ★ヒット！ダメージ！
			player_->OnCollision((Enemy*)nullptr);

			delete beam;
			it = beams_.erase(it);
		} else{
			++it;
		}
	}
}

void GameScene::GenerateEnemies(){
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	for(uint32_t i = 0; i < numBlockVirtical; ++i){
		for(uint32_t j = 0; j < numBlockHorizontal; ++j){

			// マップチップの種類を取得
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j,i);

			// ★修正：数字(10)ではなく、名前(kZako)で判定する
			if(type == MapChipType::kZako || type == MapChipType::kBoss){

				Enemy* newEnemy = new Enemy();
				Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j,i);

				// ボスかザコかを判定
				Enemy::Type enemyType = (type == MapChipType::kBoss)?Enemy::Type::kBoss:Enemy::Type::kBoss;

				newEnemy->Initialize(modelBoss_,&camera_,pos,enemyType);

				if(type == MapChipType::kBoss){
					newEnemy->SetScale({3.0f, 3.0f, 3.0f});
				} else{
					// ザコは標準サイズ
					newEnemy->SetScale({1.2f, 1.2f, 1.2f});
				}

				newEnemy->SetGameScene(this);
				enemies_.push_back(newEnemy);
			}
		}
	}
}