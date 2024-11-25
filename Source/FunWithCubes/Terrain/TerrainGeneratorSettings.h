// Made by Adam Gasior (GitHub: Adanos020)

#pragma once

#include "TerrainGeneratorSettings.generated.h"

USTRUCT(BlueprintType)
struct FTerrainGeneratorSettings
{
	GENERATED_BODY()

	// The lowest altitude the surface can reach.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 BaseAltitude = 32;

	// The highest altitude the surface can reach.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 MaxAltitude = 60;

	// Altitude at and below which water is generated if given surface level is below it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 SeaLevel = 25;

	// Thickness of the dirt layer lying on top of stone.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 DirtThickness = 4;

	// Maximum depth below water for sand to generate.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 SandDepth = 3;

	// Thickness of the bedrock layer. Everything above altitude 0 will generate in an
	// evenly distributed random pattern.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 BedrockThickness = 4;

	// Seed for the random number generator.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NoiseSeed = 12345;

	// Scales the sample coordinates for noise used to generate the terrain height map.
	// Higher values result in a denser distribution of peaks and troughs, whereas
	// lower values result in a smoother terrain.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	double TerrainScale = 0.02;

	// Scales the sample coordinates for noise used to generate caves. Higher values result
	// in smaller and more densely distributed caves, whereas lower values result in larger
	// and less densely distributed caves.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	double CaveScale = 0.05;

	// Threshold above which a generated noise value determines the presence of a cave.
	// Higher values generate larger and usually more connected caves, whereas lower values
	// generate smaller and usually less connected caves.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	double CaveThreshold = 0.4;
};
