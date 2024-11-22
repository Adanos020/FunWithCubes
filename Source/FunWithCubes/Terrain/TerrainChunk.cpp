// Made by Adam Gasior (GitHub: Adanos020)

#include "TerrainChunk.h"

#include "ProceduralMeshComponent.h"

// Sets default values
ATerrainChunk::ATerrainChunk()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	if (ensure(ProceduralMesh != nullptr))
	{
		RootComponent = ProceduralMesh;
	}
}

TArray<EVoxelType> ATerrainChunk::GenerateRandomCubes() const
{
	const int32 NumVoxels = FMath::Square(Resolution) * MaxHeight;
	TArray<EVoxelType> Voxels;
	Voxels.SetNumZeroed(NumVoxels);

	ParallelFor(NumVoxels, [&](int32 Index)
	{
		Voxels[Index] = static_cast<EVoxelType>(
			FMath::RandRange(static_cast<int32>(EVoxelType::Air), static_cast<int32>(EVoxelType::Water)));
	}, EParallelForFlags::Unbalanced);

	return Voxels;
}

TArray<EVoxelType> ATerrainChunk::GenerateTerrain() const
{
	const int32 NumVoxels = FMath::Square(Resolution) * MaxHeight;
	TArray<EVoxelType> Voxels;
	Voxels.SetNumZeroed(NumVoxels);

	const int32 NumHeights = FMath::Square(Resolution);
	TArray<int32> Heights;
	Heights.Init(MaxHeight - 1, NumHeights);
	
	for (int32 X = 0; X < Resolution; ++X)
	{
		for (int32 Y = 0; Y < Resolution; ++Y)
		{
			const int32 Height = Heights[X + (Y * Resolution)];
			for (int32 Z = 0; Z < MaxHeight; ++Z)
			{
				const int32 VoxelIndex = ChunkCoordsToVoxelIndex(X, Y, Z);
				if (Z > Height)
				{
					Voxels[VoxelIndex] = EVoxelType::Air;
				}
				else if (Z == Height)
				{
					Voxels[VoxelIndex] = EVoxelType::Grass;
				}
				else if (Height - Z <= 4)
				{
					Voxels[VoxelIndex] = EVoxelType::Dirt;
				}
				else
				{
					Voxels[VoxelIndex] = EVoxelType::Stone;
				}
			}
		}
	}

	return Voxels;
}

void ATerrainChunk::GenerateMesh(const TArray<EVoxelType>& InVoxels)
{
	check(ProceduralMesh != nullptr);
	
	const int32 NumVoxels = Resolution * Resolution * MaxHeight;
	if (InVoxels.Num() != NumVoxels)
	{
		UE_LOG(LogTemp, Warning, TEXT("Voxels array isn't of the desired length %d. Excess voxels will be ignored, and missing ones will be replaced with air."), NumVoxels);
	}
	
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<int32> Indices;
	TArray<FLinearColor> VertexColors;

	int32 VerticesAdded = 0;

	// This function assumes vertices are arranged counter-clockwise if the face is looked at from the outside.
	const auto AddSolidFace = [&](EVoxelType InVoxel, FVector InNormal, std::initializer_list<FVector> InVertices)
	{
		check(InVertices.size() == 4);
		
		const FLinearColor* MappedColor = VoxelColors.Find(InVoxel);
		const FLinearColor Color = MappedColor ? *MappedColor : FLinearColor::White;

		VertexColors.Append({ Color, Color, Color, Color });
		Normals.Append({ InNormal, InNormal, InNormal, InNormal });
		Vertices.Append(InVertices);
		Indices.Append({
			VerticesAdded + 0, VerticesAdded + 1, VerticesAdded + 2,
			VerticesAdded + 0, VerticesAdded + 2, VerticesAdded + 3,
		});

		VerticesAdded += 4;
	};

	for (int32 VoxelZ = 0; VoxelZ < MaxHeight; VoxelZ++)
	{
		for (int32 VoxelY = 0; VoxelY < Resolution; VoxelY++)
		{
			for (int32 VoxelX = 0; VoxelX < Resolution; VoxelX++)
			{
				const EVoxelType VoxelType = GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ);

				// Vertex offsets
				const double VertFront  = (VoxelX + 1) * Scale;
				const double VertBack   = (VoxelX + 0) * Scale;
				const double VertRight  = (VoxelY + 1) * Scale;
				const double VertLeft   = (VoxelY + 0) * Scale;
				const double VertTop    = (VoxelZ + 1) * Scale;
				const double VertBottom = (VoxelZ + 0) * Scale;

				if (IsVoxelSolid(VoxelType))
				{
					// Generate faces only where the neighbouring block isn't solid.
					
					// Front neighbour
					if (VoxelX == Resolution - 1 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX + 1, VoxelY, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::ForwardVector, {
							FVector(VertFront, VertRight, VertBottom),
							FVector(VertFront, VertLeft,  VertBottom),
							FVector(VertFront, VertLeft,  VertTop),
							FVector(VertFront, VertRight, VertTop),
						});
					}
					
					// Back neighbour
					if (VoxelX == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX - 1, VoxelY, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::BackwardVector, {
							FVector(VertBack, VertRight, VertBottom),
							FVector(VertBack, VertRight, VertTop),
							FVector(VertBack, VertLeft,  VertTop),
							FVector(VertBack, VertLeft,  VertBottom),
						});
					}
					
					// Right neighbour
					if (VoxelY == Resolution - 1 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY + 1, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::RightVector, {
							FVector(VertFront, VertRight, VertBottom),
							FVector(VertFront, VertRight, VertTop),
							FVector(VertBack,  VertRight, VertTop),
							FVector(VertBack,  VertRight, VertBottom),
						});
					}
					
					// Left neighbour
					if (VoxelY == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY - 1, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::LeftVector, {
							FVector(VertBack,  VertLeft, VertTop),
							FVector(VertFront, VertLeft, VertTop),
							FVector(VertFront, VertLeft, VertBottom),
							FVector(VertBack,  VertLeft, VertBottom),
						});
					}
					
					// Top neighbour
					if (VoxelZ == MaxHeight - 1 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ + 1)))
					{
						AddSolidFace(VoxelType, FVector::UpVector, {
							FVector(VertFront, VertRight, VertTop),
							FVector(VertFront, VertLeft,  VertTop),
							FVector(VertBack,  VertLeft,  VertTop),
							FVector(VertBack,  VertRight, VertTop),
						});
					}

					// Bottom neighbour
					if (VoxelZ == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ - 1)))
					{
						AddSolidFace(VoxelType, FVector::DownVector, {
							FVector(VertBack,  VertLeft,  VertBottom),
							FVector(VertFront, VertLeft,  VertBottom),
							FVector(VertFront, VertRight, VertBottom),
							FVector(VertBack,  VertRight, VertBottom),
						});
					}
				}
			}
		}
	}
	ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, {}, VertexColors, {}, false);
}

int32 ATerrainChunk::ChunkCoordsToVoxelIndex(int32 X, int32 Y, int32 Z) const
{
	return Z + (Y * MaxHeight) + (X * MaxHeight * Resolution);
}

EVoxelType ATerrainChunk::GetVoxelOrAir(const TArray<EVoxelType>& InVoxels, int32 X, int32 Y, int32 Z) const
{
	const int32 VoxelIndex = ChunkCoordsToVoxelIndex(X, Y, Z);
	if (!InVoxels.IsValidIndex(VoxelIndex))
	{
		return EVoxelType::Air;
	}
	return InVoxels[VoxelIndex];
}

