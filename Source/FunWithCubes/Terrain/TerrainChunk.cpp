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
				const double VertRight  = (VoxelX + 1) * Scale;
				const double VertLeft   = (VoxelX + 0) * Scale;
				const double VertFront  = (VoxelY + 1) * Scale;
				const double VertBack   = (VoxelY + 0) * Scale;
				const double VertTop    = (VoxelZ + 1) * Scale;
				const double VertBottom = (VoxelZ + 0) * Scale;

				if (IsVoxelSolid(VoxelType))
				{
					// Generate faces only where the neighbouring block isn't solid.
					
					// Right neighbour
					if (VoxelX == Resolution || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX + 1, VoxelY, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::RightVector, {
							FVector(VertFront, VertRight, VertBottom),
							FVector(VertFront, VertRight, VertTop),
							FVector(VertBack,  VertRight, VertTop),
							FVector(VertBack,  VertRight, VertBottom),
						});
					}
					
					// Left neighbour
					if (VoxelX == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX - 1, VoxelY, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::LeftVector, {
							FVector(VertBack,  VertLeft, VertTop),
							FVector(VertFront, VertLeft, VertTop),
							FVector(VertFront, VertLeft, VertBottom),
							FVector(VertBack,  VertLeft, VertBottom),
						});
					}
					
					// Front neighbour
					if (VoxelY == Resolution || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY + 1, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::ForwardVector, {
							FVector(VertFront, VertRight, VertBottom),
							FVector(VertFront, VertLeft,  VertBottom),
							FVector(VertFront, VertLeft,  VertTop),
							FVector(VertFront, VertRight, VertTop),
						});
					}
					
					// Back neighbour
					if (VoxelY == 0 || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY - 1, VoxelZ)))
					{
						AddSolidFace(VoxelType, FVector::BackwardVector, {
							FVector(VertBack, VertRight, VertBottom),
							FVector(VertBack, VertRight, VertTop),
							FVector(VertBack, VertLeft,  VertTop),
							FVector(VertBack, VertLeft,  VertBottom),
						});
					}
					
					// Top neighbour
					if (VoxelZ == MaxHeight || !IsVoxelSolid(GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ + 1)))
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

