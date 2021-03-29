#pragma once
//#define VALK_DBG
#include "Logger.h"

#ifdef VALK_DBG
	#define DBG_INFO(msg, ...) Logger::PushDebug(msg, ##__VA_ARGS__);
	#define DBG_CLEAR() Logger::ClearDebug();
	#define DBG_DUMP() Logger::DumpDebug();
#else
	#define DBG_INFO(msg, ...) while(false);
	#define DBG_CLEAR() while(false);
	#define DBG_DUMP() while(false);
#endif