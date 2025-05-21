#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

Vector3& operator+=(Vector3& lhv, const Vector3& rhv);
Vector3& operator-=(Vector3& lhv, const Vector3& rhv);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

// 線形補間
float Lerp(float a, float b, float t);

// easeInOut 関数（S字カーブ）
// 0.0 <= t <= 1.0 の範囲で使用
float EaseInOut(float t);

// easeInOut を使った補間
float EaseInOutLerp(float a, float b, float t);
