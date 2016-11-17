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
	Task e1;
	e1.type = EVENT_ERROR;
	e1.task_data = error;
	m_event_service->put(e1);
}

void CTPGateWay::onLog(LogData& log)
{
	Task e1;
	e1.type = EVENT_LOG;
	e1.task_data = log;
	m_event_service->put(e1);
}

void CTPGateWay::onContract(ContractData& contract)
{
	Task e1;
	e1.type = EVENT_CONTRACT;
	e1.task_data = contract;
	m_event_service->put(e1);
}

std::string CTPGateWay::getName()
{
	return gateWayName;
}

EventEngine* CTPGateWay::getEventEngine()
{
	return m_event_service;
}

void CTPGateWay::connect(std::string& userID,
						std::string& password,
						std::string& brokerID,
						std::string& mdAddress,
						std::string& tdAddress)
{
	md->connect(userID, password, brokerID, mdAddress);
	td->connect(userID, password, brokerID, tdAddress);

	m_event_service->registerHandler(EVENT_TIMER, std::bind(&CTPGateWay::query, this, std::placeholders::_1));
}

void CTPGateWay::subscribe(SubscribeReq& subReq)
{
	md->subscribe(subReq);
}

std::string CTPGateWay::sendOrder(OrderReq& orderReq)
{
	return td->sendOrder(orderReq);
}

void CTPGateWay::cancelOrder(CancelOrderReq& cancelOrderReq)
{
	td->cancelOrder(cancelOrderReq);
}

void CTPGateWay::qryAccount()
{
	if (tdConnected)
	{
		td->qryAccount();
	}
}
void CTPGateWay::qryPosition()
{
	if (tdConnected)
	{
		td->qryPosition();
	}
}
void CTPGateWay::close()
{
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