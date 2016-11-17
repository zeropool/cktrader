#include "strategyartrsi.h"
#include "servicemgr_iml.h"
#include <iostream>
#include "utils/ckdef.h"
#include <functional>

using namespace cktrader;

void on_log(Datablk& log)
{
	LogData data_log = log.cast<LogData>();
	std::cout << data_log.gateWayName << " : " << data_log.logContent << std::endl;
}

int main()
{
	ServiceMgr mgr;
	IGateway* ctp_gate = mgr.loadGateWay("ctp", "E:\\2106\\c++\\cktrader\\x64\\Debug\\CTPGateway.dll");
	if (!ctp_gate)
	{
		std::cout << "loading gateway failed" << std::endl;
		return -1;
	}
	EventEngine *pEvent = ctp_gate->getEventEngine();
	pEvent->registerHandler(EVENT_LOG, std::bind(on_log, std::placeholders::_1));

	ctp_gate->connect(std::string("036789"),
		std::string("85399386"),
		std::string("9999"),
		std::string("tcp://180.168.146.187:10010"),
		std::string("tcp://180.168.146.187:10000"));

	Sleep(5000);

	IStrategy* rsk = mgr.loadStrategy("artris","strategyAtrRsi.dll");
	if (!rsk)
	{
		std::cout << "loading strategy failed" << std::endl;
		return -2;
	}

	bool isInit = mgr.initStrategy("artris");
	if (!isInit)
	{
		std::cout << "strategy init failed" << std::endl;
		return -3;
	}

	bool isStarted = mgr.startStrategy("artris");
	if (!isStarted)
	{
		std::cout << "strategy start failed" << std::endl;
		return -4;
	}

	bool isStoped = mgr.stopStrategy("artris");
	
	getchar();
	return 0;
}