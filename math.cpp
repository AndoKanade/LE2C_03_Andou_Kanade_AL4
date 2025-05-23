#include "Math.h"

Vector3& operator+=(Vector3& lhv, const Vector3& rhv) {
	lhv.x += rhv.x;
	lhv.y += rhv.y;
	lhv.z += rhv.z;
	return lhv;
}
Vector3& operator-=(Vector3& lhv, const Vector3& rhv) {
	lhv.x -= rhv.x;
	lhv.y -= rhv.y;
	lhv.z -= rhv.z;
	return lhv;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 matrix = {}; // すべて0で初期化
	// スケール行列の設定
	matrix.m[0][0] = scale.x;
	matrix.m[1][1] = scale.y;
	matrix.m[2][2] = scale.z;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result{};

	result.m[0][0] = 1;
	result.m[3][3] = 1;

	// X軸回転に必要な部分だけ上書き
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);

	return result;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result{};

	result.m[1][1] = 1.0f;
	result.m[3][3] = 1.0f;

	result.m[0][0] = std::cos(radian);
	result.m[0][2] = -std::sin(radian);
	result.m[2][0] = std::sin(radian);
	result.m[2][2] = std::cos(radian);

	return result;
}

// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result{};

	result.m[2][2] = 1;
	result.m[3][3] = 1;

	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);

	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 matrix = {}; // すべて0で初期化
	// 単位行列の形に設定
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	// 平行移動成分を設定
	matrix.m[3][0] = translate.x;
	matrix.m[3][1] = translate.y;
	matrix.m[3][2] = translate.z;
	return matrix;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.m[row][col] += m1.m[row][k] * m2.m[k][col];
			}
		}
	}
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);

	// 回転順: Z → X → Y →（スケーリング）→ 平行移動
	Matrix4x4 rotateMatrix = Multiply(Multiply(rotateX, rotateY), rotateZ);

	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	Matrix4x4 affineMatrix = Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);

	return affineMatrix;
}

// 線形補間
float Lerp(float a, float b, float t) { return a + (b - a) * t; }

float EaseInOut(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10); // smootherstep: 0〜1
}

// easeInOut を使った補間
float EaseInOutLerp(float a, float b, float t) { return Lerp(a, b, EaseInOut(t)); }
