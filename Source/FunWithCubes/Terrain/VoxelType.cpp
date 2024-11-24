#include "VoxelType.h"

bool IsVoxelSolid(EVoxelType VoxelType)
{
	switch (VoxelType)
	{
	case EVoxelType::Bedrock:
	case EVoxelType::Dirt:
	case EVoxelType::Grass:
	case EVoxelType::Sand:
	case EVoxelType::Stone:
		return true;

	default:
		return false;
	}
}
