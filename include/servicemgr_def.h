#pragma once
#ifndef __SERVICEMGR_DEF_H__
#define __SERVICEMGR_DEF_H__

#include "utils/ckdef.h"
#include "utils/cktypes.h"
#include "gateway/tgateway_def.h"
#include "strategy/strategy_def.h"

namespace cktrader {

class IServiceMgr
{
public:
	virtual IGateway* getGateWay(std::string gateWayName) = 0;
};

}//cktrader

#endif