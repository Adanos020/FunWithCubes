// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "VoxelType.generated.h"

UENUM(BlueprintType)
enum class EVoxelType : uint8
{
	Air,
	Bedrock,
	Dirt,
	Grass,
	Stone,
	Water,
};

bool IsVoxelSolid(EVoxelType VoxelType);
