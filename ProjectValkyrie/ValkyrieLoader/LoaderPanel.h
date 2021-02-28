#pragma once
#include "ValkyrieAPI.h"
#include "AsyncTaskPool.h"
#include "Constants.h"
#include "Strings.h"

class ValkyrieLoader;
class LoaderPanel {

public:
	virtual void Draw(ValkyrieLoader& loader) = 0;

protected:
	AsyncTaskPool*              taskPool = AsyncTaskPool::Get();
	ValkyrieAPI*                api = ValkyrieAPI::Get();
};