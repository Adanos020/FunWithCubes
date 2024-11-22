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

	const auto AddFaceNormals = [&Normals](FVector InNormal)
	{
		Normals.Append({ InNormal, InNormal, InNormal, InNormal });
	};

	const auto AddFaceColors = [&VertexColors, this](EVoxelType InVoxel)
	{
		const FLinearColor* MappedColor = VoxelColors.Find(InVoxel);
		const FLinearColor Color = MappedColor ? *MappedColor : FLinearColor::White;
		VertexColors.Append({ Color, Color, Color, Color });
	};

	const auto AddFaceIndices = [&Indices, &VerticesAdded]
	{
		Indices.Append({
			VerticesAdded + 0, VerticesAdded + 1, VerticesAdded + 2,
			VerticesAdded + 0, VerticesAdded + 2, VerticesAdded + 3,
		});
	};

	for (int32 VoxelZ = 0; VoxelZ < MaxHeight; VoxelZ++)
	{
		for (int32 VoxelY = 0; VoxelY < Resolution; VoxelY++)
		{
			for (int32 VoxelX = 0; VoxelX < Resolution; VoxelX++)
			{
				const EVoxelType VoxelType = GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ);

				// Vertex offsets
				const double VertRight  = (VoxelX + 0) * Scale;
				const double VertLeft   = (VoxelX + 1) * Scale;
				const double VertFront  = (VoxelY + 0) * Scale;
				const double VertBack   = (VoxelY + 1) * Scale;
				const double VertBottom = (VoxelZ + 0) * Scale;
				const double VertTop    = (VoxelZ + 1) * Scale;

				if (IsVoxelSolid(VoxelType))
				{
					// Generate faces only where the neighbouring block isn't solid.
					
					// Left neighbour
					if (VoxelX == Resolution || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX + 1, VoxelY, VoxelZ)))
					{
						Vertices.Append({
							FVector(VertLeft, VertFront, VertBottom),
							FVector(VertLeft, VertFront, VertTop),
							FVector(VertLeft, VertBack,  VertTop),
							FVector(VertLeft, VertBack,  VertBottom),
						});
						AddFaceColors(VoxelType);
						AddFaceNormals(FVector::LeftVector);
						AddFaceIndices();
						VerticesAdded += 4;
					}
					
					// Right neighbour
					if (VoxelX == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX - 1, VoxelY, VoxelZ)))
					{
						Vertices.Append({
							FVector(VertRight, VertFront, VertBottom),
							FVector(VertRight, VertBack,  VertBottom),
							FVector(VertRight, VertBack,  VertTop),
							FVector(VertRight, VertFront, VertTop),
						});
						AddFaceColors(VoxelType);
						AddFaceNormals(FVector::RightVector);
						AddFaceIndices();
						VerticesAdded += 4;
					}
					
					// Front neighbour
					if (VoxelY == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY - 1, VoxelZ)))
					{
						Vertices.Append({
							FVector(VertLeft,  VertFront, VertBottom),
							FVector(VertRight, VertFront, VertBottom),
							FVector(VertRight, VertFront, VertTop),
							FVector(VertLeft,  VertFront, VertTop),
						});
						AddFaceColors(VoxelType);
						AddFaceNormals(FVector::ForwardVector);
						AddFaceIndices();
						VerticesAdded += 4;
					}
					
					// Back neighbour
					if (VoxelY == Resolution || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY + 1, VoxelZ)))
					{
						Vertices.Append({
							FVector(VertRight, VertBack, VertBottom),
							FVector(VertLeft,  VertBack, VertBottom),
							FVector(VertLeft,  VertBack, VertTop),
							FVector(VertRight, VertBack, VertTop),
						});
						AddFaceColors(VoxelType);
						AddFaceNormals(FVector::BackwardVector);
						AddFaceIndices();
						VerticesAdded += 4;
					}

					// Bottom neighbour
					if (VoxelZ == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ - 1)))
					{
						Vertices.Append({
							FVector(VertLeft,  VertBack,  VertBottom),
							FVector(VertRight, VertBack,  VertBottom),
							FVector(VertRight, VertFront, VertBottom),
							FVector(VertLeft,  VertFront, VertBottom),
						});
						AddFaceColors(VoxelType);
						AddFaceNormals(FVector::DownVector);
						AddFaceIndices();
						VerticesAdded += 4;
					}
					
					// Top neighbour
					if (VoxelZ == MaxHeight || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ + 1)))
					{
						Vertices.Append({
							FVector(VertLeft,  VertFront, VertTop),
							FVector(VertRight, VertFront, VertTop),
							FVector(VertRight, VertBack,  VertTop),
							FVector(VertLeft,  VertBack,  VertTop),
						});
						AddFaceColors(VoxelType);
						AddFaceNormals(FVector::UpVector);
						AddFaceIndices();
						VerticesAdded += 4;
					}
				}
			}
		}
	}
	ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, {}, VertexColors, {}, false);
}

int32 ATerrainChunk::ChunkCoordsToVoxelIndex(int32 X, int32 Y, int32 Z) const
{
	return X + (Y * Resolution) + (Z * FMath::Square(Resolution));
}

EVoxelType ATerrainChunk::GetVoxelOrAir(const TArray<EVoxelType>& InVoxels, int32 X, int32 Y, int32 Z) const
{
	const int32 VoxelIndex = ChunkCoordsToVoxelIndex(X, Y, Z);
	if (VoxelIndex >= InVoxels.Num())
	{
		return EVoxelType::Air;
	}
	return InVoxels[VoxelIndex];
}

