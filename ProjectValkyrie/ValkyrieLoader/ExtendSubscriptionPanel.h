#pragma once
#include "LoaderPanel.h"
#include "Constants.h"

class ExtendSubscriptionPanel : public LoaderPanel {
public:
	virtual void     Draw(ValkyrieLoader& loader);

	char name[Constants::INPUT_TEXT_SIZE];
	char code[Constants::INPUT_TEXT_SIZE];
};