#include "GameScene.h"
#include "Math.h"
#include "ParticleManager.h"
#include "BossEffectSystem.h"

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
	delete modelEnemy_;
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
	mapChipField_->LoadMapChipCsv("Resources/MapChip.csv");
	modelBlock_ = Model::CreateFromOBJ("block");
	GenerateBlocks();

	// プレイヤー生成
	player_ = new Player();
	modelPlayer_ = Model::CreateFromOBJ("player");
	modelAttack_ = Model::CreateFromOBJ("attack_effect");
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(26,18);

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
	modelEnemy_ = Model::CreateFromOBJ("kakashi");
	modelBoss_ = Model::CreateFromOBJ("enemy");

	// 1. カカシ (練習用)
	Enemy* scarecrow = new Enemy();
	Vector3 posScarecrow = mapChipField_->GetMapChipPositionByIndex(20,18);
	scarecrow->Initialize(modelEnemy_,&camera_,posScarecrow,Enemy::Type::kScarecrow);
	scarecrow->SetGameScene(this);
	enemies_.push_back(scarecrow);

	// 2. ボス (本番用)
	Enemy* boss = new Enemy();
	Vector3 posBoss = mapChipField_->GetMapChipPositionByIndex(35,18);
	boss->Initialize(modelBoss_,&camera_,posBoss,Enemy::Type::kBoss);
	boss->SetGameScene(this);
	enemies_.push_back(boss);

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

	// --- 1. 自キャラ vs 敵キャラ ---
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

	// --- 2. ビーム vs 敵 ---
	for(auto itBeam = beams_.begin(); itBeam != beams_.end(); ++itBeam){
		Beam* beam = *itBeam;

		for(Enemy* enemy : enemies_){
			// 簡易的な球判定（距離の2乗チェック）
			Vector3 posA = beam->GetWorldPosition();
			Vector3 posB = enemy->GetWorldPosition();
			Vector3 diff = posA - posB;
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float r = beam->GetRadius() + 1.0f; // 敵の半径(仮) + ビーム半径

			if(distSq <= r * r){
				// ヒット処理
				beam->OnCollision();         // ビーム消滅
				enemy->OnCollision(player_); // 敵へダメージ通知（引数は攻撃元情報としてplayerを渡す）

				// ヒットエフェクト発生
				CreateEffect(posB);
				break; // ビームは貫通せず、1体に当たったら消える
			}
		}
	}
}