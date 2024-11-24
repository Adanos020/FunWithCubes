// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkLoader.generated.h"

UCLASS()
class FUNWITHCUBES_API AChunkLoader : public AActor
{
	GENERATED_BODY()

public:
	AChunkLoader();

	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Chunk Properties")
	TSubclassOf<class ATerrainChunk> ChunkClass;

	// Distance (units: chunk count) within which chunks will be generated in a square shape around the player.
	UPROPERTY(EditAnywhere, Category = "Chunk Properties")
	int32 RenderDistance = 5;
	
	UPROPERTY(Transient)
	double ChunkWidth = 3200.0;
	UPROPERTY(Transient)
	int32 MaxChunkHeight = 64;
	
	// Coordinates of the chunk in which the player pawn currently is.
	UPROPERTY(Transient)
	FIntVector2 LastPlayerChunk = { 0, 0 };

	// Maps a loaded chunk to its coordinates.
	UPROPERTY(Transient)
	TMap<FIntVector2, class ATerrainChunk*> LoadedChunks;
};
