// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelType.h"
#include "TerrainGeneratorSettings.h"

#include "TerrainChunk.generated.h"

UCLASS()
class FUNWITHCUBES_API ATerrainChunk : public AActor
{
	GENERATED_BODY()

protected:
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
			const TMap<EVoxelType, FLinearColor>& Colors,
			std::initializer_list<FVector> InVertices
		);
	};
	
public:
	ATerrainChunk();

	UFUNCTION(BlueprintCallable, CallInEditor)
	TArray<EVoxelType> GenerateTerrain(const FTerrainGeneratorSettings& Settings) const;
	
	UFUNCTION(BlueprintCallable)
	void GenerateMesh(const TArray<EVoxelType>& InVoxels);
	
	int32 GetResolution() const { return Resolution; }
	int32 GetMaxHeight() const { return MaxHeight; }
	double GetScale() const { return Scale; }

protected: // Details buttons
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Chunk")
	void RandomSeed();
	
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
	
	// Colours associated with each block type.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EVoxelType, FLinearColor> VoxelColors;

	// Number of blocks in the chunk along both the X and Y axis.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Resolution = 32;

	// Length of the side of each block.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Scale = 1.0;

	// Height limit of the chunk.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHeight = 64;

	// Whether chunk edges should appear in the chunk mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowChunkBorderFaces = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTerrainGeneratorSettings TerrainGeneratorSettings;
};
