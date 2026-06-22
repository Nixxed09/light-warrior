#include "LuminousWitnessFallbackData.h"

bool ULuminousWitnessFallbackData::GetLineForBeat(ELuminousWitnessBeat Beat, FLuminousWitnessFallbackLine& OutLine) const
{
    const FLuminousWitnessFallbackLine* BestLine = nullptr;

    for (const FLuminousWitnessFallbackLine& Line : Lines)
    {
        if (Line.Beat != Beat)
        {
            continue;
        }

        if (!BestLine || Line.Priority > BestLine->Priority)
        {
            BestLine = &Line;
        }
    }

    if (!BestLine)
    {
        return false;
    }

    OutLine = *BestLine;
    return true;
}
