#include "GameScene.h"
#include "ImGui.h"
using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete blockModel_;
	for (WorldTransform* worldTransformBlock : worldTransformBlocks_) {
		delete worldTransformBlock;
	}
	worldTransformBlocks_.clear();
}

void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く

	textureHandle_ = TextureManager::Load("ashe.jpg");
	model_ = Model::Create();
	blockModel_ = Model::Create();

	const uint32_t knumBlockHorizontal = 20;
	const float kBlockWidth = 2.0f;
	worldTransformBlocks_.resize(knumBlockHorizontal);

	for (uint32_t i = 0; i < knumBlockHorizontal; i++) {
		WorldTransform* worldTransformBlocks_[i] = new WorldTransform();
		worldTransformBlocks_[i]->Initialize();
		worldTransformBlocks_[i]->translation.x = kBlockWidth * i;
		worldTransformBlocks_[i]->translation.y = 0.0f;
	}

	worldTransform_.Initialize();
	camera_.Initialize();

	PrimitiveDrawer::GetInstance()->SetViewProjection(&camera_);

	player_ = new Player();
	player_->Initialize(model_, textureHandle_, &camera_);
}

void GameScene::Update() {
	// ここにインゲームの更新処理を書く
	player_->Update();
	for (WorldTransform* worldTransformBlock : worldTransformBlocks_) {

		/// 3次元のアフェイン変換行列を作成する
		worldTransformBlock->matWorld_ = Multiply(Multiply(worldTransformBlock->scale_, worldTransformBlock->translation_), worldTransformBlock->rotation_);
	}
}

void GameScene::Draw() {
	// ここにインゲームの描画処理を書く
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());
	player_->Draw();
	Model::PostDraw();
}
