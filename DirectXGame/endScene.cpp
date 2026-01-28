#include "endScene.h"
#include "Math.h"
#include <numbers>

EndScene::~EndScene(){
	delete modelPlayer_;
	delete modelTitle_;
	delete fade_;
	if(Audio::GetInstance()->IsPlaying(bgmHandle_)){
		Audio::GetInstance()->StopWave(bgmHandle_);
	}
}

void EndScene::Initialize(){

	modelTitle_ = Model::CreateFromOBJ("endFont",true);
	modelPlayer_ = Model::CreateFromOBJ("player");

	// カメラ初期化
	camera_.Initialize();

	const float kPlayerTitle = 2.0f;

	worldTransformTitle_.Initialize();

	worldTransformTitle_.scale_ = {kPlayerTitle, kPlayerTitle, kPlayerTitle};

	const float kPlayerScale = 10.0f;

	worldTransformPlayer_.Initialize();

	worldTransformPlayer_.scale_ = {kPlayerScale, kPlayerScale, kPlayerScale};

	worldTransformPlayer_.rotation_.y = 0.95f * std::numbers::pi_v<float>;

	worldTransformPlayer_.translation_.x = -2.0f;

	worldTransformPlayer_.translation_.y = -10.0f;

	fade_ = new Fade();
	fade_->Initialize();

	fade_->Start(Fade::Status::FadeIn,1.0f);

	bgmDataHandle_ = Audio::GetInstance()->LoadWave("BGM/endScene.wav");
	seDataHandle_ = Audio::GetInstance()->LoadWave("SE/enter.wav");

}

void EndScene::Update(){

	switch(phase_){
	case Phase::kFadeIn:
		fade_->Update();

		if(fade_->IsFinished()){
			phase_ = Phase::kMain;
			PlayBgm();
		}
		break;
	case Phase::kMain:
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

	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_,kTimeTitleMove);

	float angle = counter_ / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformTitle_.translation_.y = std::sin(angle) + 10.0f;

	camera_.TransferMatrix();

	// アフィン変換～DirectXに転送(タイトル座標)
	WorldTransformUpdate(worldTransformTitle_);

	// アフィン変換～DirectXに転送（プレイヤー座標）
	WorldTransformUpdate(worldTransformPlayer_);
}

void EndScene::Draw(){

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	modelTitle_->Draw(worldTransformTitle_,camera_);
	modelPlayer_->Draw(worldTransformPlayer_,camera_);

	Model::PostDraw();

	fade_->Draw();
}

void EndScene::PlayBgm(){
	if(!isPlayBgm_){
		bgmHandle_ = Audio::GetInstance()->PlayWave(bgmDataHandle_,true,0.2f);
		isPlayBgm_ = true;
	}
}

void EndScene::PlaySe(){
	if(!isPlaySe_){
		seHandle_ = Audio::GetInstance()->PlayWave(seDataHandle_,false,0.3f);
		isPlaySe_ = true;
	}
}
