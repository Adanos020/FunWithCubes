#include "FPerlinNoise3D.h"

FPerlinNoise3D::FPerlinNoise3D(int32 Seed)
{
	GenerateNoise(Seed);
}

void FPerlinNoise3D::GenerateNoise(int32 Seed)
{
	FRandomStream Rng(Seed);
	for (int i = 0; i < 512; i++)
	{
		Permutations[i] = Rng.RandRange(0, 255);
	}
}

double FPerlinNoise3D::GetValue(FVector Position) const
{
	const int32 X = static_cast<int32>(FMath::Floor(Position.X)) & 0xFF;
	const int32 Y = static_cast<int32>(FMath::Floor(Position.Y)) & 0xFF;
	const int32 Z = static_cast<int32>(FMath::Floor(Position.Z)) & 0xFF;
	Position.X = FMath::Fractional(Position.X);
	Position.Y = FMath::Fractional(Position.Y);
	Position.Z = FMath::Fractional(Position.Z);
	const double U = Fade(Position.X);
	const double V = Fade(Position.Y);
	const double W = Fade(Position.Z);
	const int32 A = Permutations[X] + Y;
	const int32 AA = Permutations[A] + Z;
	const int32 AB = Permutations[A + 1] + Z;
	const int32 B = Permutations[X + 1] + Y;
	const int32 BA = Permutations[B] + Z;
	const int32 BB = Permutations[B + 1] + Z;
	const double Noise = FMath::Lerp(
		FMath::Lerp(
			FMath::Lerp(
				Grad(Permutations[AA], FVector(Position.X    , Position.Y, Position.Z)),
				Grad(Permutations[BA], FVector(Position.X - 1, Position.Y, Position.Z)),
				U
			),
			FMath::Lerp(
				Grad(Permutations[AB], FVector(Position.X    , Position.Y - 1, Position.Z)),
				Grad(Permutations[BB], FVector(Position.X - 1, Position.Y - 1, Position.Z)),
				U
			),
			V
		),
		FMath::Lerp(
			FMath::Lerp(
				Grad(Permutations[AA + 1], FVector(Position.X    , Position.Y, Position.Z - 1)),
				Grad(Permutations[BA + 1], FVector(Position.X - 1, Position.Y, Position.Z - 1)),
				U
			),
			FMath::Lerp(
				Grad(Permutations[AB + 1], FVector(Position.X    , Position.Y - 1, Position.Z - 1)),
				Grad(Permutations[BB + 1], FVector(Position.X - 1, Position.Y - 1, Position.Z - 1)),
				U
			),
			V
		),
		W
	);
	return (Noise + 1) * 0.5;
}

double FPerlinNoise3D::Fade(double T)
{
	return T * T * T * (T * (T * 6.0 - 15.0) + 10.0);
}

double FPerlinNoise3D::Grad(int32 Hash, FVector Position)
{
	Hash = Hash & 0xF;
	const double U = (Hash < 8) ? Position.X : Position.Y;
	const double V = (Hash < 4) ? Position.Y :
	                 (Hash == 12 || Hash == 14) ? Position.X : Position.Z;
	return ((Hash & 1) == 0 ? U : -U) + ((Hash & 2) == 0 ? V : -V);
}
