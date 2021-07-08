#pragma once

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/detail/convertible.hpp>

using namespace boost::python;

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "imgui/imgui.h"

#include "AsyncTaskPool.h"
#include "AsyncTask.h"
#include "UserInfo.h"

#include "Offsets.h"
#include "Memory.h"
#include "Logger.h"
#include "Benchmark.h"

#include "OffsetScanner.h"
#include "GameRenderer.h"

#include "SpellCast.h"
#include "GameUnit.h"

