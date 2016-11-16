#include "CTPGateway.h"
#include "ctpmd.h"
#include "ctptd.h"
#include "eventservice/eventengine.h"

namespace cktrader {

IGateway* g_ctpGateway = nullptr;

CK_EXPORTS IGateway* CreateGateway(const char* x)
{
	if (!g_ctpGateway)
	{
		g_ctpGateway = new CTPGateWay(x);
	}

	return g_ctpGateway;
}

CK_EXPORTS int ReleaseGateway(IGateway*p)
{
	if (g_ctpGateway)
	{
		delete g_ctpGateway;
		g_ctpGateway = nullptr;
	}

	return CK_TRUE;
}

CTPGateWay::CTPGateWay(std::string gateWayName)
{
	this->m_event_service = new EventEngine();	

	mdConnected = false;
	tdConnected = false;

	this->gateWayName = gateWayName;

	md = new CtpMd(m_event_service, this);
	td = new CtpTd(m_event_service, this);

	this->m_event_service->startEngine();
}
CTPGateWay::~CTPGateWay()
{
	mdConnected = false;
	tdConnected = false;

	md->close();
	delete md;
	md = nullptr;

	td->close();
	delete td;
	td = nullptr;
}

void CTPGateWay::onTick(TickData& tick)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_TICK;
	e1.task_data = tick;
	m_event_service->put(e1);

	Task e2;
	e2.type = std::string(EVENT_TICK) + tick.tSymbol;
	e2.task_data = tick;
	m_event_service->put(e2);
}

void CTPGateWay::onTrade(TradeData& trade)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_TRADE;
	e1.task_data = trade;
	m_event_service->put(e1);

	Task e2;
	e2.type = std::string(EVENT_TRADE) + trade.tSymbol;
	e2.task_data = trade;
	m_event_service->put(e2);
}

void CTPGateWay::onOrder(OrderData& order)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_ORDER;
	e1.task_data = order;
	m_event_service->put(e1);

	Task e2;
	e2.type = std::string(EVENT_ORDER) + order.tSymbol;
	e2.task_data = order;
	m_event_service->put(e2);
}

void CTPGateWay::onPosition(PositionData& position)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_POSITION;
	e1.task_data = position;
	m_event_service->put(e1);

	Task e2;
	e2.type = std::string(EVENT_POSITION) + position.tSymbol;
	e2.task_data = position;
	m_event_service->put(e2);
}

void CTPGateWay::onAccount(AccountData& account)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_ACCOUNT;
	e1.task_data = account;
	m_event_service->put(e1);

	Task e2;
	e2.type = std::string(EVENT_ACCOUNT) + account.tAccountID;
	e2.task_data = account;
	m_event_service->put(e2);
}

void CTPGateWay::onError(ErrorData& error)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_ERROR;
	e1.task_data = error;
	m_event_service->put(e1);
}

void CTPGateWay::onLog(LogData& log)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_LOG;
	e1.task_data = log;
	m_event_service->put(e1);
}

void CTPGateWay::onContract(ContractData& contract)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	Task e1;
	e1.type = EVENT_CONTRACT;
	e1.task_data = contract;
	m_event_service->put(e1);
}

std::string CTPGateWay::getName()
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);
	return gateWayName;
}

EventEngine* CTPGateWay::getEventEngine()
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);
	return m_event_service;
}

void CTPGateWay::connect(std::string& userID,
						std::string& password,
						std::string& brokerID,
						std::string& mdAddress,
						std::string& tdAddress)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	md->connect(userID, password, brokerID, mdAddress);
	td->connect(userID, password, brokerID, tdAddress);

	m_event_service->registerHandler(EVENT_TIMER, std::bind(&CTPGateWay::query, this, std::placeholders::_1));
}

void CTPGateWay::subscribe(SubscribeReq& subReq)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	md->subscribe(subReq);
}

std::string CTPGateWay::sendOrder(OrderReq& orderReq)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	return td->sendOrder(orderReq);
}

void CTPGateWay::cancelOrder(CancelOrderReq& cancelOrderReq)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	td->cancelOrder(cancelOrderReq);
}

void CTPGateWay::qryAccount()
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	if (tdConnected)
	{
		td->qryAccount();
	}
}
void CTPGateWay::qryPosition()
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	if (tdConnected)
	{
		td->qryPosition();
	}
}
void CTPGateWay::close()
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	if (mdConnected)
	{
		md->close();
	}
	if (tdConnected)
	{
		td->close();
	}
}

void CTPGateWay::query(Datablk& notUse)
{
	std::unique_lock<std::recursive_mutex> lck(the_mutex);

	qryCount++;
	if (qryCount == 1)
	{
		qryAccount();
	}
	if (qryCount == 2)
	{
		qryPosition();
	}

	if (qryCount>qryTrigger)
	{
		qryCount = 0;
	}
}

}//cktrader