// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "CoreMinimal.h"
#include "VoxelType.h"
#include "GameFramework/Actor.h"
#include "TerrainChunk.generated.h"

UCLASS()
class FUNWITHCUBES_API ATerrainChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrainChunk();

	UFUNCTION(BlueprintCallable)
	void GenerateMesh(const TArray<EVoxelType>& InVoxels);

protected: // Helper functions
	int32 ChunkCoordsToVoxelIndex(int32 X, int32 Y, int32 Z) const;
	EVoxelType GetVoxelOrAir(const TArray<EVoxelType>& InVoxels, int32 X, int32 Y, int32 Z) const;
	
protected: // Data
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	class UProceduralMeshComponent* ProceduralMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TMap<EVoxelType, FLinearColor> VoxelColors;

	// How many blocks there are in the chunk along both the X and Y axis.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Resolution = 32;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Scale = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxHeight = 128;
};
