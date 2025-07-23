#include "GameScene.h"
#include "ImGui.h"

using namespace KamataEngine;

void GameScene::CreateEffect(const KamataEngine::Vector3& position) {
	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}

GameScene::~GameScene() {

	delete deathParticles_;
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	delete player_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
			worldTransformBlock = nullptr;
		}
	}
	delete modelParticle_;
	delete modelDeathParticle_;
	delete modelEnemy_;
	delete modelPlayer_;
	delete modelPlayerAttack_;
	delete modelBlock_;
	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;
	delete cameraController_;
	delete fade_;
}
void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く

	model_ = Model::Create();
	modelBlock_ = Model::CreateFromOBJ("block", true);

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/MapChip.csv");
	GenerateBlocks();

	player_ = new Player();

	// プレイヤーモデル
	modelPlayer_ = Model::CreateFromOBJ("player");
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);

	modelPlayerAttack_ = Model::CreateFromOBJ("attack_effect");

	player_->SetMapChipField(mapChipField_);

	player_->Initialize(modelPlayer_, modelPlayerAttack_, &camera_, playerPosition);
	// worldTransform_.Initialize();

	camera_.farZ = 1000.0f; // カメラの遠くの描画距離
	camera_.Initialize();
	PrimitiveDrawer::GetInstance()->SetViewProjection(&camera_);

	debugCamera_ = new DebugCamera(1280, 720);

	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_->Initialize(&camera_, modelSkydome_);

	cameraController_ = new CameraController();

	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);

	modelEnemy_ = Model::CreateFromOBJ("enemy");

	for (int32_t i = 0; i < 2; i++) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(14 + i * 2, 18);
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

	modelDeathParticle_ = Model::CreateFromOBJ("deathParticle");

	phase_ = Phase::kPlay;

	modelParticle_ = Model::CreateFromOBJ("particle");
	HitEffect::SetModel(modelParticle_);
	HitEffect::SetCamera(&camera_);
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:
		// 02_12 13枚目 if文から中身まで全部実装
		// Initialize関数のいきなりパーティクル発生処理は消す
		if (player_->IsDead()) {
			// 死亡演出
			phase_ = Phase::kDeath;

			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticle_, &camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		break;
	}
}

void GameScene::GenerateBlocks() {

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {

		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::Update() {
	// ここにインゲームの更新処理を書く

	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	ChangePhase();

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		skydome_->Update();
		cameraController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//	cameraController->Update();

		// 自キャラの更新
		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		// UpdateCamera();
#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			// フラグをトグル
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// UpdateBlocks();
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}
		break;
	case Phase::kPlay:
		skydome_->Update();
		cameraController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		cameraController->Update();

		// 自キャラの更新
		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		CheckAllCollisions();
		break;
	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
		}

		skydome_->Update();
		cameraController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		UpdateCamera();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		if (deathParticles_) {
			deathParticles_->Update();
		}

		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		skydome_->Update();
		cameraController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		UpdateCamera();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		break;
	}
}

void GameScene::Draw() {

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dオブジェクト描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// 自キャラの描画
	if (!player_->IsDead()) {
		player_->Draw();
	}

	// 天球描画
	skydome_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	Model::PostDraw();

	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	// スプライト描画後処理
	Sprite::PostDraw();
}

void GameScene::CheckAllCollisions() {

	// 判定対象1と2の座標
	AABB aabb1, aabb2;

#pragma region 自キャラと敵キャラの当たり判定
	{
		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾全ての当たり判定
		for (Enemy* enemy : enemies_) {

			// コリジョン無効の敵はスキップ
			if (enemy->IsCollisionDisabled())
				continue;

			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_->OnCollision(enemy);
				// 敵弾の衝突時コールバックを呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion
}
