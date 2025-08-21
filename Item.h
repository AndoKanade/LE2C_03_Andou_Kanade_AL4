#pragma once
#include "KamataEngine.h"
#include "Math.h"

class Player;

class Item {
public:
	Vector3 position;
	AABB GetAABB() const {
		AABB aabb;
		aabb.min = position - Vector3{0.5f, 0.5f, 0.5f};
		aabb.max = position + Vector3{0.5f, 0.5f, 0.5f};
		return aabb;
	}

	bool IsCollisionDisabled() const { return isCollected_; }

	void OnCollision(Player* player) {

		(void)player;
		isCollected_ = true;
		// ここでプレイヤーに効果を与える（スコア加算など）
		
	}

private:
	bool isCollected_ = false;
};
