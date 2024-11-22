// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "CoreMinimal.h"
#include "VoxelType.h"
#include "GameFramework/Actor.h"
#include "TerrainChunk.generated.h"

USTRUCT(BlueprintType)
struct FTerrainGeneratorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 MinAltitude = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = -1, UIMin = -1))
	int32 MaxAltitude = -1; // -1 sets the maximum to the chunk's MaxHeight - 1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = -1, UIMin = -1))
	int32 SeaLevel = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NoiseSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TerrainScale = 0.01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CaveScale = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CaveThreshold = 0.8;
};

struct FMeshSegmentData
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<int32> Indices;
	TArray<FLinearColor> VertexColors;
	int32 VertexCount = 0;

	void AddFace(
		EVoxelType InVoxel,
		FVector InNormal,
		const TMap<EVoxelType, FLinearColor>& VoxelColors,
		std::initializer_list<FVector> InVertices
	);
};

UCLASS()
class FUNWITHCUBES_API ATerrainChunk : public AActor
{
	GENERATED_BODY()

public:
	ATerrainChunk();

	UFUNCTION(BlueprintCallable)
	TArray<EVoxelType> GenerateTerrain(FTerrainGeneratorSettings Settings) const;
	
	UFUNCTION(BlueprintCallable)
	void GenerateMesh(const TArray<EVoxelType>& InVoxels);
	
protected: // Helper functions
	int32 ChunkCoordsToVoxelIndex(int32 X, int32 Y, int32 Z) const;
	EVoxelType GetVoxelOrAir(const TArray<EVoxelType>& InVoxels, int32 X, int32 Y, int32 Z) const;
	
protected: // Data
	UPROPERTY(EditDefaultsOnly)
	class UProceduralMeshComponent* ProceduralMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* TerrainMaterial = nullptr;
	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* WaterMaterial = nullptr;

	// Colours associated with each block type.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TMap<EVoxelType, FLinearColor> VoxelColors;

	// How many blocks there are in the chunk along both the X and Y axis.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 Resolution = 32;

	// How big the blocks are.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Scale = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 MaxHeight = 128;
};
