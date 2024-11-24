// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "TerrainGeneratorSettings.generated.h"

USTRUCT(BlueprintType)
struct FTerrainGeneratorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 BaseAltitude = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 MaxAltitude = 60;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 SeaLevel = 25;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 DirtThickness = 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 BedrockThickness = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NoiseSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	double TerrainScale = 0.02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	double CaveScale = 0.05;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	double CaveThreshold = 0.4;
};
