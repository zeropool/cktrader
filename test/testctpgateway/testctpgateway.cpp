// testctpgateway.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "utils/ckdef.h"
#include "utils/cktypes.h"
#include "utils/datablk.h"
#include "utils/dllHelper.h"
#include "gateway/tgateway_def.h"

#include <iostream>
#include <functional>

using namespace cktrader;

typedef IGateway* (*CreateGateway)(const char*);

void on_log(Datablk& log)
{
	LogData data_log = log.cast<LogData>();
	std::cout << data_log.gateWayName<<" : "<<data_log.logContent << std::endl;
}

void on_tick(Datablk& tick)
{
	TickData data_tick = tick.cast<TickData>();
	std::cout << data_tick.symbol << " : " << data_tick.askPrice1 << std::endl;
}

int main()
{
	CDllHelper *_dll = new CDllHelper("E:\\2106\\c++\\cktrader\\x64\\Debug\\CTPGateway.dll");
	CreateGateway pfunc = _dll->GetProcedure<CreateGateway>("CreateGateway");
	IGateway* gate;
	if (!pfunc)
	{
		return -2;
	}
	gate = pfunc("ctp");
	if (!gate)
	{
		return -1;
	}
	EventEngine* pEvent = gate->getEventEngine();
	if (!pEvent)
	{
		return -3;
	}
	pEvent->registerHandler(EVENT_LOG, std::bind(on_log, std::placeholders::_1));
	pEvent->registerHandler(EVENT_TICK, std::bind(on_tick, std::placeholders::_1));

	gate->connect(std::string("036789"),
		std::string("85399386"),
		std::string("9999"),
		std::string("tcp://180.168.146.187:10010"),
		std::string("tcp://180.168.146.187:10000"));

	Sleep(1000);

	SubscribeReq sub;
	sub.symbol = "rb1701";
	gate->subscribe(sub);
	sub.symbol = "IF1703";
	gate->subscribe(sub);

	getchar();
    return 0;
}

