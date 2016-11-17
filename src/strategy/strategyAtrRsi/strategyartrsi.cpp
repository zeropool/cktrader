#include "strategyartrsi.h"

#include <assert.h>
#include <iostream>
#include <functional>

IStrategy* g_strategy_art_rsi = nullptr;

CK_EXPORTS IStrategy* CreateStrategy(IServiceMgr*x, const char*name)
{
	if (!g_strategy_art_rsi)
	{
		g_strategy_art_rsi = new StrategyAtrRsi(x,name);
	}

	return g_strategy_art_rsi;
}

CK_EXPORTS int ReleaseStrategy(IStrategy*p)
{
	if (g_strategy_art_rsi)
	{
		delete g_strategy_art_rsi;
		g_strategy_art_rsi = nullptr;
	}

	return CK_TRUE;
}

StrategyAtrRsi::StrategyAtrRsi(IServiceMgr* x, const char* name)
{
	pGateway = x->getGateWay("ctp");
}

StrategyAtrRsi::~StrategyAtrRsi()
{

}

bool StrategyAtrRsi::onInit()
{
	EventEngine* pEvent = pGateway->getEventEngine();
	pEvent->registerHandler(EVENT_TICK, std::bind(&StrategyAtrRsi::onTick, this, std::placeholders::_1));
	pEvent->registerHandler(EVENT_ORDER, std::bind(&StrategyAtrRsi::onOrder, this, std::placeholders::_1));
	pEvent->registerHandler(EVENT_TRADE, std::bind(&StrategyAtrRsi::onTrade, this, std::placeholders::_1));
	pEvent->registerHandler(EVENT_LOG, std::bind(&StrategyAtrRsi::onLog, this, std::placeholders::_1));
	pEvent->registerHandler(EVENT_TIMER, std::bind(&StrategyAtrRsi::timer, this, std::placeholders::_1));

	SubscribeReq sub;
	sub.symbol = "rb1701";
	pGateway->subscribe(sub);
	std::cout << "rsi:: init"<<std::endl;
	return true;
}

bool StrategyAtrRsi::onStart()
{
	std::cout << "rsi:: start"<<std::endl;
	return true;
}

bool StrategyAtrRsi::onStop()
{
	std::cout << "rsi:: stop"<<std::endl;
	return true;
}

void StrategyAtrRsi::onTick(Datablk& tick)
{
	TickData tick_data = tick.cast<TickData>();
	std::cout << "rsi:: tick" << std::endl;
	std::cout << tick_data.symbol <<" : "<<tick_data.askPrice1<< std::endl;
}

void StrategyAtrRsi::onOrder(Datablk&  order)
{
	std::cout << "rsi:: order" << std::endl;
}

void StrategyAtrRsi::onTrade(Datablk&  trade)
{
	std::cout << "rsi:: trade" << std::endl;
}

void StrategyAtrRsi::onLog(Datablk&  log)
{
	LogData log_data = log.cast<LogData>();

	std::cout << "rsi:: log" << std::endl;
	std::cout << log_data.logContent << std::endl;
}

void StrategyAtrRsi::timer(Datablk& tick)
{
	std::cout << "timer" << std::endl;
}