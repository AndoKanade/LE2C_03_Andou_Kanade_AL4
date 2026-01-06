#include "GameScene.h"
#include "Math.h"

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

	// 寿命
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

void GameScene::CreateEffect(const Vector3& position){

	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}

GameScene::~GameScene(){

	delete sprite_;
	delete model_;
	delete modelArrow_;
	// GameScene::~GameScene 内

	delete modelEnemy_;
	delete modelBoss_; // ★追加: 忘れずに消す
	delete modelBlock_;
	for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
		for(WorldTransform* worldTransformBlock : worldTransformBlockLine){
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;
	for(Enemy* enemy : enemies_){
		delete enemy;
	}

	delete deathParticles_;
	delete modelDeathEffect_;

	for(HitEffect* hitEffect : hitEffects_){
		delete hitEffect;
	}

	for(Beam* beam : beams_){
		delete beam;
	}
	delete modelBeam_;
}

void GameScene::Initialize(){

	modelArrow_ = Model::CreateFromOBJ("arrow");

	Vector3 arrowPosition = mapChipField_->GetMapChipPositionByIndex(26,9);

	textureHandle_ = TextureManager::Load("sample.png");
	sprite_ = Sprite::Create(textureHandle_,{100, 50});
	model_ = Model::Create();
	worldTransform_.Initialize();

	camera_.Initialize();

	modelBlock_ = Model::CreateFromOBJ("block");

	debugCamera_ = new DebugCamera(WinApp::kWindowWidth,WinApp::kWindowHeight);

	skydome_ = new Skydome();

	modelSkydome_ = Model::CreateFromOBJ("SkyDome",true);
	skydome_->Initialize(&camera_,modelSkydome_);

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/MapChip.csv");
	GenerateBlocks();

	player_ = new Player();

	modelPlayer_ = Model::CreateFromOBJ("player");
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(26,18);
	modelAttack_ = Model::CreateFromOBJ("attack_effect"); // 02_07 スライド5枚目
	player_->SetMapChipField(mapChipField_);

	player_->Initialize(modelPlayer_,modelAttack_,&camera_,playerPosition);

	cameraController_ = new CameraController(); // 生成
	cameraController_->Initialize(&camera_);    // 初期化
	cameraController_->SetTarget(player_);      // 追従対象セット
	cameraController_->Reset();                 // リセット

	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);

	// 敵モデル読み込み
	modelEnemy_ = Model::CreateFromOBJ("kakashi");
	modelBoss_ = Model::CreateFromOBJ("enemy");
	// modelEnemy2_ = Model::CreateFromOBJ("door"); 
	// modelEnemy3_ = Model::CreateFromOBJ("arrow");

	// ==========================================
	// ★変更: 敵（カカシとボス）の生成
	// ==========================================

	// 1. カカシ (練習用・緑色・無敵)
	Enemy* scarecrow = new Enemy();
	// プレイヤーより少し手前（左側）に配置
	Vector3 posScarecrow = mapChipField_->GetMapChipPositionByIndex(20,18);
	// ★初期化時にタイプを指定 (Enemy.hの変更が必要)
	scarecrow->Initialize(modelEnemy_,&camera_,posScarecrow,Enemy::Type::kScarecrow);
	scarecrow->SetGameScene(this);
	enemies_.push_back(scarecrow);

	// 2. ボス (クリア条件・赤色・HP10)
	Enemy* boss = new Enemy();
	// プレイヤーより奥（右側）に配置
	Vector3 posBoss = mapChipField_->GetMapChipPositionByIndex(35,18);
	// ★初期化時にタイプを指定
	boss->Initialize(modelBoss_,&camera_,posBoss,Enemy::Type::kBoss);
	boss->SetGameScene(this);
	enemies_.push_back(boss);


	modelDeathEffect_ = Model::CreateFromOBJ("deathParticle");

	modelParticle_ = Model::CreateFromOBJ("particle");
	phase_ = Phase::kFadeIn;

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn,1.0f);

	HitEffect::SetModel(modelParticle_);
	HitEffect::SetCamera(&camera_);

	modelBeam_ = Model::CreateFromOBJ("sphere");
}

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
		break;
	}
}

void GameScene::GenerateBlocks(){

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for(uint32_t i = 0; i < numBlockVirtical; ++i){
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
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

void GameScene::Update(){

	hitEffects_.remove_if([](HitEffect* hitEffect){
		if(hitEffect->IsDead()){
			delete hitEffect;

			return true;
		}
		return false;
		});

	enemies_.remove_if([](Enemy* enemy){
		if(enemy->IsDead()){
			delete enemy;
			return true;
		}
		return false;
		});

	beams_.remove_if([](Beam* beam){
		if(beam->IsDead()){
			delete beam;
			return true;
		}
		return false;
		});


	ChangePhase();

	switch(phase_){
	case Phase::kFadeIn:
		fade_->Update();
		if(fade_->IsFinished()){
			fade_->Start(Fade::Status::FadeOut,1.0f);
			phase_ = Phase::kPlay;
		}

		skydome_->Update();
		cameraController_->Update();
		// 自キャラの更新
		player_->Update();

		for(Enemy* enemy : enemies_){
			enemy->Update();
		}

		for(HitEffect* hitEffect : hitEffects_){
			hitEffect->Update();
		}
		// カメラの処理
		if(isDebugCameraActive_){
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else{
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
			for(WorldTransform*& worldTransformBlock : worldTransformBlockLine){

				if(!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}
		Vector3 arrowPosition = mapChipField_->GetMapChipPositionByIndex(26,9);

		for(HitEffect* hitEffect : hitEffects_){
			hitEffect->Update();
		}
		break;
	case Phase::kPlay:
		skydome_->Update();
		cameraController_->Update();
		// 自キャラの更新
		player_->Update();

		for(Enemy* enemy : enemies_){
			enemy->Update();
		}


		// ==========================================
		// ★追加: ビーム発射チェック
		// ==========================================
		if(player_->IsShotBeam()){
			Beam* newBeam = new Beam();

			// プレイヤーの位置
			Vector3 startPos = player_->GetWorldPosition();
			// startPos.y += 1.0f; // 少し高さを上げるならここを調整

			// プレイヤーの向きに合わせて飛ばす
			float rotY = player_->GetWorldTransform().rotation_.y;
			Vector3 velocity = {sinf(rotY), 0, cosf(rotY)};

			// 速度調整
			velocity *= 0.5f; // 弾速

			newBeam->Initialize(modelBeam_,startPos,velocity);
			beams_.push_back(newBeam);
		}

		// ★追加: ビームの更新
		for(Beam* beam : beams_){
			beam->Update();
		}

		// ==========================================
		// ★変更: クリア判定 (ボス死亡チェック)
		// ==========================================
		{
			bool isBossAlive = false;
			for(Enemy* enemy : enemies_){
				// ボスタイプが生きていればフラグを立てる
				if(enemy->GetType() == Enemy::Type::kBoss){
					isBossAlive = true;
					break;
				}
			}

			// ボスが見つからなかった = 倒された
			if(!isBossAlive){
				phase_ = Phase::kFadeOut;
			}
		}

		// カメラの処理
		if(isDebugCameraActive_){
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else{
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
			for(WorldTransform*& worldTransformBlock : worldTransformBlockLine){

				if(!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		CheckAllCollisions();

		for(HitEffect* hitEffect : hitEffects_){
			hitEffect->Update();
		}
		break;
	case Phase::kDeath:
		if(deathParticles_ && deathParticles_->IsFinished()){
			phase_ = Phase::kFadeOut;
		}

		skydome_->Update();
		cameraController_->Update();
		for(Enemy* enemy : enemies_){
			enemy->Update();
		}

		if(deathParticles_){
			deathParticles_->Update();
		}

		for(HitEffect* hitEffect : hitEffects_){
			hitEffect->Update();
		}
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if(fade_->IsFinished()){
			finished_ = true;
		}

		skydome_->Update();
		cameraController_->Update();
		for(Enemy* enemy : enemies_){
			enemy->Update();
		}

		for(HitEffect* hitEffect : hitEffects_){
			hitEffect->Update();
		}

		break;
	}
}

void GameScene::Draw(){

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	// 自キャラの描画
	if(!player_->IsDead())
		player_->Draw();

	// 天球描画
	skydome_->Draw();

	// ブロックの描画
	for(std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_){
		for(WorldTransform*& worldTransformBlock : worldTransformBlockLine){
			if(!worldTransformBlock)
				continue;

			modelBlock_->Draw(*worldTransformBlock,camera_);
		}
	}
	for(Enemy* enemy : enemies_){
		enemy->Draw();
	}
	for(Beam* beam : beams_){
		beam->Draw(camera_);
	}


	if(deathParticles_){
		deathParticles_->Draw();
	}

	for(HitEffect* hitEffect : hitEffects_){
		hitEffect->Draw();
	}

	Model::PostDraw();

	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	// スプライト描画後処理
	Sprite::PostDraw();

	// 02_13 28枚目
	fade_->Draw();
}

void GameScene::CheckAllCollisions(){

	AABB aabb1,aabb2;

#pragma region 自キャラと敵キャラの当たり判定
	{
		aabb1 = player_->GetAABB();

		for(Enemy* enemy : enemies_){

			if(enemy->IsCollisionDisabled())
				continue;

			aabb2 = enemy->GetAABB();

			if(IsCollision(aabb1,aabb2)){
				player_->OnCollision(enemy);
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion

	// ==========================================
	// ★追加: ビーム vs 敵
	// ==========================================
	for(auto itBeam = beams_.begin(); itBeam != beams_.end(); ++itBeam){
		Beam* beam = *itBeam;

		for(Enemy* enemy : enemies_){
			// 簡易的な球判定（距離チェック）
			Vector3 posA = beam->GetWorldPosition();
			Vector3 posB = enemy->GetWorldPosition();

			// 差分ベクトルを一旦変数に入れる
			Vector3 diff = posA - posB;

			// その場で「2乗の長さ」を計算する (x*x + y*y + z*z)
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float r = beam->GetRadius() + 1.0f; // 敵の半径(仮)

			if(distSq <= r * r){
				// ヒット！
				beam->OnCollision(); // ビーム消滅
				enemy->OnCollision(player_); // 敵ダメージ

				// 派手にエフェクトを出す！
				CreateEffect(posB);

				break; // ビームは1体に当たったら消える
			}
		}
	}


#pragma region 自キャラとアイテムの当たり判定

#pragma endregion
}