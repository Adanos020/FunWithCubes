#include "VoxelType.h"

bool IsVoxelSolid(EVoxelType VoxelType)
{
	switch (VoxelType)
	{
	case EVoxelType::Dirt:
	case EVoxelType::Stone:
		return true;

	default:
		return false;
	}
}

bool IsVoxelSemiTransparent(EVoxelType VoxelType)
{
	switch (VoxelType)
	{
	case EVoxelType::Water:
		return true;

	default:
		return false;
	}
}
