#include "Lot.hpp"


void Lot::AddWafer(Uuid wId)
{
    waferIds_.push_back(wId);
}

void Lot::RemoveWafer(Uuid wId)
{
    waferIds_.remove(wId);
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Lot)
