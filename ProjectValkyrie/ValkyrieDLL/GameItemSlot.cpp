#include "GameItemSlot.h"

object GameItemSlot::GetItemPy()
{
	return object(ptr(item));
}

object GameItemSlot::GetActivePy()
{
	return object(ptr(active));
}
