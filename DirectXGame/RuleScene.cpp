#include "RuleScene.h"

RuleScene::~RuleScene(){
	// スプライトとフェードの削除
	delete sprite_;
	delete fade_;
	if(Audio::GetInstance()->IsPlaying(bgmHandle_)){
		Audio::GetInstance()->StopWave(bgmHandle_);
	}
}

void RuleScene::Initialize(){
	// 画像の読み込み
	textureHandle_ = TextureManager::Load("ruleScene.png");

	// スプライト生成 (画面サイズに合わせて生成)
	// ※WinApp::kWindowWidth 等が使えない場合は直接数値を指定してください (例: 1280, 720)
	sprite_ = Sprite::Create(textureHandle_,{0.0f,0.0f});

	// フェード初期化
	fade_ = new Fade();
	fade_->Initialize();

	// フェードイン開始
	fade_->Start(Fade::Status::FadeIn,1.0f);

	bgmDataHandle_ = Audio::GetInstance()->LoadWave("BGM/ruleScene.wav");
	seDataHandle_ = Audio::GetInstance()->LoadWave("SE/enter.wav");
}

void RuleScene::Update(){

	switch(phase_){
	case Phase::kFadeIn:
		fade_->Update();

		if(fade_->IsFinished()){
			phase_ = Phase::kMain;
			PlayBgm();
		}
		break;
	case Phase::kMain:
		// スペースキーでルール画面を閉じる（タイトルへ戻る等の処理用）
		if(Input::GetInstance()->PushKey(DIK_SPACE)){
			fade_->Start(Fade::Status::FadeOut,1.0f);
			phase_ = Phase::kFadeOut;
			PlaySe();
		}
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if(fade_->IsFinished()){
			finished_ = true;
		}
		break;
	}
}

void RuleScene::Draw(){

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// スプライト描画前処理
	Sprite::PreDraw(commandList);

	// 画像の描画
	if(sprite_){
		sprite_->Draw();
	}

	// スプライト描画後処理
	Sprite::PostDraw();

	// フェードの描画 (最前面)
	fade_->Draw();
}

void RuleScene::PlayBgm(){
	if(!isPlayBgm_){
		bgmHandle_ = Audio::GetInstance()->PlayWave(bgmDataHandle_,true,0.2f);
		isPlayBgm_ = true;
	}
}

void RuleScene::PlaySe(){
	if(!isPlaySe_){
		seHandle_ = Audio::GetInstance()->PlayWave(seDataHandle_,false,0.3f);
		isPlaySe_ = true;
	}
}