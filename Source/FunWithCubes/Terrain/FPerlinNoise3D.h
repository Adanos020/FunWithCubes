// Perlin noise based on Ken Perlin's implementation: https://cs.nyu.edu/~perlin/noise/

#pragma once

struct FPerlinNoise3D
{
public:
	FPerlinNoise3D(int32 Seed = FMath::Rand());

	void GenerateNoise(int32 Seed = FMath::Rand());
	double GetValue(FVector Position) const;

private:
	static double Fade(double T);
	static double Grad(int32 Hash, FVector Position);
	
private:
	int32 Permutations[512];
};
