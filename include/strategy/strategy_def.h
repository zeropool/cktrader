#pragma once
#ifndef __STRATEGY_DEF_H__
#define __STRATEGY_DEF_H__

#include "utils/ckdef.h"
#include "utils/cktypes.h"

namespace cktrader {

class IStrategy
{
public:
	virtual bool onInit() = 0;
	virtual bool onStart() = 0;
	virtual bool onStop() = 0;
};

}//cktrader

#endif