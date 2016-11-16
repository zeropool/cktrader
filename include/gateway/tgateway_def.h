#pragma once
#ifndef __TGATEWAY_H__
#define __TGATEWAY_H__

#include <string>
#include <mutex>

#include "utils/ckdef.h"
#include "utils/cktypes.h"
#include "eventservice/eventengine.h"

namespace cktrader {

class IGateway
{
public:
	virtual void onTick(TickData& tick)=0;
	virtual void onTrade(TradeData& trade)=0;
	virtual void onOrder(OrderData& order)=0;
	virtual void onPosition(PositionData& position)=0;
	virtual void onAccount(AccountData& account)=0;
	virtual void onError(ErrorData& error)=0;
	virtual void onLog(LogData& log)=0;
	virtual void onContract(ContractData& contract)=0;

	virtual std::string getName()=0;
	virtual EventEngine* getEventEngine()=0;

	virtual void connect(std::string& userID, std::string& password, std::string& brokerID, std::string& mdAddress, std::string& tdAddress) = 0;
	virtual void subscribe(SubscribeReq& subReq) = 0;
	virtual std::string sendOrder(OrderReq& orderReq) = 0;
	virtual void cancelOrder(CancelOrderReq& cancelOrderReq) = 0;
	virtual void qryAccount() = 0;
	virtual void qryPosition() = 0;
	virtual void close() = 0;
};

}//cktrader
#endif
