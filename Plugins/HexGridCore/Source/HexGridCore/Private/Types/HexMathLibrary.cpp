#include "Types/HexMathLibrary.h"


const FHexCoord UHexMathLibrary::DirectionOffsets[6] = {
    FHexCoord(1, 0),   // Right
    FHexCoord(1, -1),  // TopRight
    FHexCoord(0, -1),  // TopLeft
    FHexCoord(-1, 0),  // Left
    FHexCoord(-1, 1),  // BottomLeft
    FHexCoord(0, 1)    // BottomRight
};

FHexCoord UHexMathLibrary::Add(const FHexCoord& A, const FHexCoord& B)
{
    return FHexCoord(A.Q + B.Q, A.R + B.R);
}

FHexCoord UHexMathLibrary::GetDirectionOffset(EHexDirection Direction)
{
    return DirectionOffsets[static_cast<uint8>(Direction)];
}

FHexCoord UHexMathLibrary::GetNeighbour(const FHexCoord& Hex, EHexDirection Direction)
{
    return Add(Hex, GetDirectionOffset(Direction));
}

EHexDirection UHexMathLibrary::GetFacingDirection(const FHexCoord& From, const FHexCoord& To)
{
    const int32 DQ = To.Q - From.Q;
    const int32 DR = To.R - From.R;
    const int32 DS = -DQ - DR;

    if (DQ == 0 && DR == 0)
    {
        return EHexDirection::Right;
    }

    static constexpr int32 Dirs[6][3] = {
        { 1,  0, -1}, // Right
        { 1, -1,  0}, // TopRight
        { 0, -1,  1}, // TopLeft
        {-1,  0,  1}, // Left
        {-1,  1,  0}, // BottomLeft
        { 0,  1, -1}  // BottomRight
    };

    int32 MaxDot = TNumericLimits<int32>::Lowest();
    int32 BestDirIndex = 0;

    for (int32 Index = 0; Index < 6; ++Index)
    {
        const int32 Dot = DQ * Dirs[Index][0] + DR * Dirs[Index][1] + DS * Dirs[Index][2];
        if (Dot > MaxDot)
        {
            MaxDot = Dot;
            BestDirIndex = Index;
        }
        else if (Dot == MaxDot && Index == (BestDirIndex + 1) % 6)
        {
            BestDirIndex = Index;
        }
    }

    return static_cast<EHexDirection>(BestDirIndex);
}

float UHexMathLibrary::GetDirectionYaw(EHexDirection Direction)
{
    static constexpr float DirectionYaws[6] = {
        0.0f,    // Right
        -60.0f,  // TopRight
        -120.0f, // TopLeft
        180.0f,  // Left
        120.0f,  // BottomLeft
        60.0f    // BottomRight
    };

    return DirectionYaws[static_cast<uint8>(Direction)];
}

FRotator UHexMathLibrary::GetDirectionRotation(EHexDirection Direction, float YawOffset)
{
    return FRotator(0.0f, GetDirectionYaw(Direction) + YawOffset, 0.0f);
}

TArray<FHexCoord> UHexMathLibrary::GetAllNeighbours(const FHexCoord& Hex)
{
    TArray<FHexCoord> Result;
    for (int i = 0; i < 6; ++i)
    {
        Result.Add(GetNeighbour(Hex, static_cast<EHexDirection>(i)));
    }
    return Result;
}

int32 UHexMathLibrary::GetDistance(const FHexCoord& A, const FHexCoord& B)
{
    int32 dQ = A.Q - B.Q;
    int32 dR = A.R - B.R;
    int32 dS = (-A.Q - A.R) - (-B.Q - B.R); // S = -Q - R

    return FMath::Max3(FMath::Abs(dQ), FMath::Abs(dR), FMath::Abs(dS));
}

FVector2D UHexMathLibrary::HexToLocalSpace(const FHexCoord& Hex, float HexSize)
{
    const float SQRT3 = FMath::Sqrt(3.0f);
    float x = HexSize * SQRT3 * (Hex.Q + Hex.R / 2.0f);
    float y = HexSize * 1.5f * Hex.R;
    return FVector2D(x, y);
}

FHexCoord UHexMathLibrary::WorldToHex(const FVector2D& Point, float HexSize)
{
    const float SQRT3 = FMath::Sqrt(3.0f);
    float q = (SQRT3 / 3.0f * Point.X - 1.0f / 3.0f * Point.Y) / HexSize;
    float r = (2.0f / 3.0f * Point.Y) / HexSize;

    float s = -q - r;

    int32 rx = FMath::RoundToInt(q);
    int32 ry = FMath::RoundToInt(r);
    int32 rz = FMath::RoundToInt(s);

    float xDiff = FMath::Abs(rx - q);
    float yDiff = FMath::Abs(ry - r);
    float zDiff = FMath::Abs(rz - s);

    if (xDiff > yDiff && xDiff > zDiff)
    {
        rx = -ry - rz;
    }
    else if (yDiff > zDiff)
    {
        ry = -rx - rz;
    }

    return FHexCoord(rx, ry);
}



