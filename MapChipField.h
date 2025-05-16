#pragma once
#include <KamataEngine.h>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace KamataEngine;

enum class MapChipType {
	kBlank,
	kBlock,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline uint32_t kNumBlockVirtical = 20;
	static inline uint32_t kNumBlockHorizontal = 100;

public:
	MapChipData mapChipData_;
	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetnumBlockVirtivcal();
	uint32_t GetnumBlockHorizontal();
};
