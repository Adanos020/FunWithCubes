// Made by Adam Gasior (GitHub: Adanos020)

#include "ChunkLoader.h"

#include "TerrainChunk.h"

AChunkLoader::AChunkLoader()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChunkLoader::BeginPlay()
{
	Super::BeginPlay();
	
	if (ensure(ChunkClass != nullptr))
	{
		ATerrainChunk* DefaultChunk = ChunkClass->GetDefaultObject<ATerrainChunk>();

		ChunkWidth = DefaultChunk->GetScale() * static_cast<double>(DefaultChunk->GetResolution());
		MaxChunkHeight = DefaultChunk->GetMaxHeight();
	}
}

void AChunkLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (APlayerController* Controller = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = Controller->GetPawnOrSpectator())
		{
			const FVector PawnLocation = Pawn->GetActorLocation();
			const FIntVector CurrentPlayerChunk3D(PawnLocation / ChunkWidth);
			const FIntVector2 CurrentPlayerChunk2D = { CurrentPlayerChunk3D.X, CurrentPlayerChunk3D.Y };
			if (CurrentPlayerChunk2D != LastPlayerChunk || LoadedChunks.IsEmpty())
			{
				LastPlayerChunk = CurrentPlayerChunk2D;

				FIntVector2 CurrentChunkCoord;
				for (CurrentChunkCoord.X = CurrentPlayerChunk2D.X - RenderDistance;
					CurrentChunkCoord.X < CurrentPlayerChunk2D.X + RenderDistance;
					CurrentChunkCoord.X++) 
				{
					for (CurrentChunkCoord.Y = CurrentPlayerChunk2D.Y - RenderDistance;
						CurrentChunkCoord.Y < CurrentPlayerChunk2D.Y + RenderDistance;
						CurrentChunkCoord.Y++)
					{
						if (!LoadedChunks.Contains(CurrentChunkCoord))
						{
							const FVector ChunkLocation = {
								CurrentChunkCoord.X * ChunkWidth,
								CurrentChunkCoord.Y * ChunkWidth,
								0.0,
							};

							FActorSpawnParameters SpawnParams;
							SpawnParams.bHideFromSceneOutliner = true;
							
							if (
								ATerrainChunk* NewChunk = GetWorld()->SpawnActor<ATerrainChunk>(
									ChunkClass, ChunkLocation, FRotator::ZeroRotator, SpawnParams)
							) {
								LoadedChunks.Add(CurrentChunkCoord, NewChunk);
							}
						}
					}
				}
			}

			TArray<FIntVector2> LoadedChunkCoords;
			LoadedChunks.GetKeys(LoadedChunkCoords);
			for (const FIntVector2 ChunkCoord : LoadedChunkCoords)
			{
				if (
					FMath::Abs(ChunkCoord.X - LastPlayerChunk.X) > RenderDistance
					|| FMath::Abs(ChunkCoord.Y - LastPlayerChunk.Y) > RenderDistance
				) {
					GetWorld()->DestroyActor(LoadedChunks[ChunkCoord]);
					LoadedChunks.Remove(ChunkCoord);
				}
			}
		}
	}
}

