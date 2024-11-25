// Made by Adam Gasior (GitHub: Adanos020)

#include "TerrainChunk.h"

#include "FPerlinNoise3D.h"
#include "ProceduralMeshComponent.h"

// This function assumes vertices are arranged counter-clockwise if the face is looked at from the outside.
void ATerrainChunk::FMeshSegmentData::AddFace(
	EVoxelType InVoxel,
	FVector InNormal,
	const TMap<EVoxelType, FLinearColor>& Colors,
	std::initializer_list<FVector> InVertices
) {
	check(InVertices.size() == 4);
	
	const FLinearColor* MappedColor = Colors.Find(InVoxel);
	const FLinearColor Color = MappedColor ? *MappedColor : FLinearColor::White;

	VertexColors.Append({ Color, Color, Color, Color });
	Normals.Append({ InNormal, InNormal, InNormal, InNormal });
	Vertices.Append(InVertices);
	Indices.Append({
		VertexCount + 0, VertexCount + 1, VertexCount + 2,
		VertexCount + 0, VertexCount + 2, VertexCount + 3,
	});

	VertexCount += 4;
}

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

TArray<EVoxelType> ATerrainChunk::GenerateTerrain(const FTerrainGeneratorSettings& Settings) const
{
	FRandomStream BedrockRng(Settings.NoiseSeed);
	FPerlinNoise3D TerrainNoise(Settings.NoiseSeed);
	FPerlinNoise3D CaveNoise(Settings.NoiseSeed * 13 / 11);

	// Actual number of voxels generated per horizontal dimension is `Resolution + 2`. The reason for the extra
	// padding is that I want to have access to noise values in neighbouring chunks in order to prevent generating
	// unnecessary faces on chunk borders. The actual displayed chunk will still have a width of `Resolution`.
	const int32 PaddedResolution = Resolution + 2;
	const int32 NumVoxels = FMath::Square(PaddedResolution) * MaxHeight;
	TArray<EVoxelType> Voxels;
	Voxels.SetNumZeroed(NumVoxels);

	const int32 NumHeights = FMath::Square(PaddedResolution);
	TArray<int32> Heights;
	Heights.SetNumUninitialized(NumHeights);

	auto ChunkLocation = FIntVector(GetActorLocation() / Scale);

	// Account for padding.
	ChunkLocation.X -= 1;
	ChunkLocation.Y -= 1;
	
	// Generate heights
	for (int32 X = 0; X < PaddedResolution; ++X)
	{
		for (int32 Y = 0; Y < PaddedResolution; ++Y)
		{
			const FVector P = FVector(ChunkLocation.X + X, ChunkLocation.Y + Y, 0) * Settings.TerrainScale;
			const FVector Q = {
				TerrainNoise.GetValue(P + FVector(0.0, 0.0, 0.0)),
				TerrainNoise.GetValue(P + FVector(5.2, 1.3, 0.0)),
				0,
			};
			const FVector R = {
				TerrainNoise.GetValue(P + (4.0 * Q) + FVector(1.7, 9.2, 0.0)),
				TerrainNoise.GetValue(P + (4.0 * Q) + FVector(8.3, 2.8, 0.0)),
				0,
			};
			const double NoiseValue = TerrainNoise.GetValue(P + (4.0 * R));
			const int32 Height = Settings.BaseAltitude + (NoiseValue * (Settings.MaxAltitude - Settings.BaseAltitude));
			Heights[X + (Y * PaddedResolution)] = Height;
		}
	}

	// Generate voxels
	for (int32 X = 0; X < PaddedResolution; ++X)
	{
		for (int32 Y = 0; Y < PaddedResolution; ++Y)
		{
			const int32 Height = Heights[X + (Y * PaddedResolution)];
			for (int32 Z = 0; Z < MaxHeight; ++Z)
			{
				const int32 VoxelIndex = ChunkCoordsToVoxelIndex(X, Y, Z);
				if (Z > Height)
				{
					if (Z > Settings.SeaLevel)
					{
						Voxels[VoxelIndex] = EVoxelType::Air;
					}
					else
					{
						Voxels[VoxelIndex] = EVoxelType::Water;
					}
				}
				else if (Z == Height)
				{
					if (Z <= Settings.SeaLevel && Z > Settings.SeaLevel - Settings.SandDepth)
					{
						Voxels[VoxelIndex] = EVoxelType::Sand;
					}
					else if (Z < Settings.SeaLevel - Settings.SandDepth)
					{
						Voxels[VoxelIndex] = EVoxelType::Dirt;
					}
					else
					{
						Voxels[VoxelIndex] = EVoxelType::Grass;
					}
				}
				else if (Height - Z <= Settings.DirtThickness)
				{
					Voxels[VoxelIndex] = EVoxelType::Dirt;
				}
				else if (Z == 0 || (Z < Settings.BedrockThickness - 1 && BedrockRng.RandRange(0, 100) < 50))
				{
					Voxels[VoxelIndex] = EVoxelType::Bedrock;
				}
				else
				{
					Voxels[VoxelIndex] = EVoxelType::Stone;
				}
			}
		}
	}

	// Carve out caves
	for (int32 X = 0; X < PaddedResolution; ++X)
	{
		for (int32 Y = 0; Y < PaddedResolution; ++Y)
		{
			for (int32 Z = 0; Z < MaxHeight; ++Z)
			{
				const double Noise = CaveNoise.GetValue((FVector(ChunkLocation) + FVector(X, Y, Z)) * Settings.CaveScale);

				// Avoid removing bedrock, water, and solid blocks neighbouring with water (except from above).
				if (
					Noise >= Settings.CaveThreshold
					&& GetVoxelOrAir(Voxels, X,     Y,     Z    ) != EVoxelType::Bedrock
					&& GetVoxelOrAir(Voxels, X,     Y,     Z    ) != EVoxelType::Water
					&& GetVoxelOrAir(Voxels, X,     Y,     Z + 1) != EVoxelType::Water
					&& GetVoxelOrAir(Voxels, X,     Y + 1, Z    ) != EVoxelType::Water
					&& GetVoxelOrAir(Voxels, X,     Y - 1, Z    ) != EVoxelType::Water
					&& GetVoxelOrAir(Voxels, X - 1, Y,     Z    ) != EVoxelType::Water
					&& GetVoxelOrAir(Voxels, X + 1, Y,     Z    ) != EVoxelType::Water
				) {
					Voxels[ChunkCoordsToVoxelIndex(X, Y, Z)] = EVoxelType::Air;
				}
			}
		}
	}
	
	return Voxels;
}

void ATerrainChunk::GenerateMesh(const TArray<EVoxelType>& InVoxels)
{
	check(ProceduralMesh != nullptr);
	
	const int32 NumVoxels = FMath::Square(Resolution + 2) * MaxHeight;
	if (InVoxels.Num() != NumVoxels)
	{
		UE_LOG(LogTemp, Warning, TEXT("Voxels array isn't of the desired length %d. Excess voxels will be ignored, and missing ones will be replaced with air."), NumVoxels);
	}

	FMeshSegmentData TerrainMeshData;
	FMeshSegmentData WaterMeshData;

	const auto AddFacesForBlock = [this, &InVoxels](
		FMeshSegmentData& MeshSegmentData,
		EVoxelType VoxelType,
		FIntVector VoxelPosition,
		const auto& NeighbourCondition
	) {
		// Vertex offsets
		double VertTopOffset = 0.0;
		if (VoxelType == EVoxelType::Water)
		{
			if (GetVoxelOrAir(InVoxels, VoxelPosition.X, VoxelPosition.Y, VoxelPosition.Z + 1) != EVoxelType::Water)
			{
				VertTopOffset = 0.1;
			}
		}
		
		const double VertFront  = Scale * (VoxelPosition.X - 0);
		const double VertBack   = Scale * (VoxelPosition.X - 1);
		const double VertRight  = Scale * (VoxelPosition.Y - 0);
		const double VertLeft   = Scale * (VoxelPosition.Y - 1);
		const double VertTop    = Scale * ((VoxelPosition.Z - 0) - VertTopOffset);
		const double VertBottom = Scale * (VoxelPosition.Z - 1);
		
		// Generate faces only where the neighbouring block isn't solid.
					
		// Front neighbour
		if (
			(VoxelPosition.X == Resolution && bShowChunkBorderFaces)
			|| NeighbourCondition(GetVoxelOrAir(InVoxels, VoxelPosition.X + 1, VoxelPosition.Y, VoxelPosition.Z))
		) {
			MeshSegmentData.AddFace(VoxelType, FVector::ForwardVector, VoxelColors, {
				FVector(VertFront, VertRight, VertBottom),
				FVector(VertFront, VertLeft,  VertBottom),
				FVector(VertFront, VertLeft,  VertTop),
				FVector(VertFront, VertRight, VertTop),
			});
		}
		
		// Back neighbour
		if (
			(VoxelPosition.X == 1 && bShowChunkBorderFaces)
			|| NeighbourCondition(GetVoxelOrAir(InVoxels, VoxelPosition.X - 1, VoxelPosition.Y, VoxelPosition.Z))
		) {
			MeshSegmentData.AddFace(VoxelType, FVector::BackwardVector, VoxelColors, {
				FVector(VertBack, VertRight, VertBottom),
				FVector(VertBack, VertRight, VertTop),
				FVector(VertBack, VertLeft,  VertTop),
				FVector(VertBack, VertLeft,  VertBottom),
			});
		}
		
		// Right neighbour
		if (
			(VoxelPosition.Y == Resolution && bShowChunkBorderFaces)
			|| NeighbourCondition(GetVoxelOrAir(InVoxels, VoxelPosition.X, VoxelPosition.Y + 1, VoxelPosition.Z))
		) {
			MeshSegmentData.AddFace(VoxelType, FVector::RightVector, VoxelColors, {
				FVector(VertFront, VertRight, VertBottom),
				FVector(VertFront, VertRight, VertTop),
				FVector(VertBack,  VertRight, VertTop),
				FVector(VertBack,  VertRight, VertBottom),
			});
		}
		
		// Left neighbour
		if (
			(VoxelPosition.Y == 1 && bShowChunkBorderFaces)
			|| NeighbourCondition(GetVoxelOrAir(InVoxels, VoxelPosition.X, VoxelPosition.Y - 1, VoxelPosition.Z))
		) {
			MeshSegmentData.AddFace(VoxelType, FVector::LeftVector, VoxelColors, {
				FVector(VertBack,  VertLeft, VertTop),
				FVector(VertFront, VertLeft, VertTop),
				FVector(VertFront, VertLeft, VertBottom),
				FVector(VertBack,  VertLeft, VertBottom),
			});
		}
		
		// Top neighbour
		if (
			(VoxelPosition.Z == MaxHeight - 1 && bShowChunkBorderFaces)
			|| NeighbourCondition(GetVoxelOrAir(InVoxels, VoxelPosition.X, VoxelPosition.Y, VoxelPosition.Z + 1))
		) {
			MeshSegmentData.AddFace(VoxelType, FVector::UpVector, VoxelColors, {
				FVector(VertFront, VertRight, VertTop),
				FVector(VertFront, VertLeft,  VertTop),
				FVector(VertBack,  VertLeft,  VertTop),
				FVector(VertBack,  VertRight, VertTop),
			});
		}

		// Bottom neighbour
		if (
			(VoxelPosition.Z == 0 && bShowChunkBorderFaces)
			|| NeighbourCondition(GetVoxelOrAir(InVoxels, VoxelPosition.X, VoxelPosition.Y, VoxelPosition.Z - 1))
		) {
			MeshSegmentData.AddFace(VoxelType, FVector::DownVector, VoxelColors, {
				FVector(VertBack,  VertLeft,  VertBottom),
				FVector(VertFront, VertLeft,  VertBottom),
				FVector(VertFront, VertRight, VertBottom),
				FVector(VertBack,  VertRight, VertBottom),
			});
		}
	};

	for (int32 VoxelZ = 0; VoxelZ < MaxHeight; VoxelZ++)
	{
		for (int32 VoxelY = 1; VoxelY < Resolution + 1; VoxelY++)
		{
			for (int32 VoxelX = 1; VoxelX < Resolution + 1; VoxelX++)
			{
				const EVoxelType VoxelType = GetVoxelOrAir(InVoxels, VoxelX, VoxelY, VoxelZ);
				
				if (IsVoxelSolid(VoxelType))
				{
					AddFacesForBlock(
						TerrainMeshData,
						VoxelType,
						FIntVector(VoxelX, VoxelY, VoxelZ),
						[](EVoxelType V) { return !IsVoxelSolid(V); }
					);
				}
				else if (VoxelType == EVoxelType::Water)
				{
					AddFacesForBlock(
						WaterMeshData,
						VoxelType,
						FIntVector(VoxelX, VoxelY, VoxelZ),
						[](EVoxelType V) { return V == EVoxelType::Air; }
					);
				}
			}
		}
	}
	
	ProceduralMesh->CreateMeshSection_LinearColor(
		0,
		TerrainMeshData.Vertices,
		TerrainMeshData.Indices,
		TerrainMeshData.Normals,
		{},
		TerrainMeshData.VertexColors,
		{},
		false
	);
	ProceduralMesh->CreateMeshSection_LinearColor(
		1,
		WaterMeshData.Vertices,
		WaterMeshData.Indices,
		WaterMeshData.Normals,
		{},
		WaterMeshData.VertexColors,
		{},
		false
	);

	ProceduralMesh->SetMaterial(0, TerrainMaterial);
	ProceduralMesh->SetMaterial(1, WaterMaterial);
}

void ATerrainChunk::RandomSeed()
{
	TerrainGeneratorSettings.NoiseSeed = FMath::Rand();
	const TArray<EVoxelType> ChunkData = GenerateTerrain(TerrainGeneratorSettings);
	GenerateMesh(ChunkData);
}

void ATerrainChunk::OnConstruction(const FTransform& Transform)
{
	const TArray<EVoxelType> ChunkData = GenerateTerrain(TerrainGeneratorSettings);
	GenerateMesh(ChunkData);
	
	Super::OnConstruction(Transform);
}

int32 ATerrainChunk::ChunkCoordsToVoxelIndex(int32 X, int32 Y, int32 Z) const
{
	return Z + (Y * MaxHeight) + (X * MaxHeight * (Resolution + 2));
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

