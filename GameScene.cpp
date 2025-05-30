#include "GameScene.h"
#include "ImGui.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete modelBlock_;
	delete playerModel_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}

	worldTransformBlocks_.clear();

	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;
}

void GameScene::GenerateBlocks() {
	const uint32_t NumBlockVirtical = mapChipField_->GetnumBlockVirtivcal();
	const uint32_t NumBlockHorizontal = mapChipField_->GetnumBlockHorizontal();

	worldTransformBlocks_.resize(NumBlockVirtical);

	for (uint32_t i = 0; i < NumBlockVirtical; i++) {
		worldTransformBlocks_[i].resize(NumBlockHorizontal);
	}

	for (uint32_t i = 0; i < NumBlockVirtical; i++) {
		for (uint32_t j = 0; j < NumBlockHorizontal; j++) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();

				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く

	model_ = Model::Create();
	modelBlock_ = Model::CreateFromOBJ("block", true);

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/MapChip.csv");
	GenerateBlocks();

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_ = new Player();
	playerModel_ = Model::CreateFromOBJ("player", true);
	player_->SetMapChipField(mapChipField_);
	player_->Initialize(playerModel_, &camera_, playerPosition);
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
}

void GameScene::Update() {
	// ここにインゲームの更新処理を書く

	player_->Update();
	skydome_->Update();
	cameraController_->Update();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			/// 3次元のアフェイン変換行列を作成する
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			worldTransformBlock->TransferMatrix();
		}
	}

	debugCamera_->Update();
#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		// isDebugCameraActive_ = true;

		if (!isDebugCameraActive_) {
			isDebugCameraActive_ = true;
		} else {
			isDebugCameraActive_ = false;
		}
	}

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

#endif
}

void GameScene::Draw() {
	// ここにインゲームの描画処理を書く
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());
	player_->Draw();
	skydome_->Draw();
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	Model::PostDraw();
}
