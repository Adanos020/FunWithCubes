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
	int32 BaseAltitude = 32;

	// -1 sets the maximum to the chunk's MaxHeight - 1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = -1, UIMin = -1))
	int32 MaxAltitude = 60;
	
	// -1 sets the maximum to the chunk's MaxHeight - 1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = -1, UIMin = -1))
	int32 SeaLevel = 25;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 DirtThickness = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NoiseSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TerrainScale = 0.02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CaveScale = 0.05;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CaveThreshold = 0.4;
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

	UFUNCTION(BlueprintCallable, CallInEditor)
	TArray<EVoxelType> GenerateTerrain(FTerrainGeneratorSettings Settings) const;
	
	UFUNCTION(BlueprintCallable)
	void GenerateMesh(const TArray<EVoxelType>& InVoxels);

protected: // Function overrides
	virtual void OnConstruction(const FTransform& Transform) override;
	
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
	
	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* WaterMaterialInstanceDynamic = nullptr;
	
	// Colours associated with each block type.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EVoxelType, FLinearColor> VoxelColors;

	// How many blocks there are in the chunk along both the X and Y axis.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Resolution = 32;

	// How big the blocks are.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHeight = 64;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTerrainGeneratorSettings TerrainGeneratorSettings;
};
