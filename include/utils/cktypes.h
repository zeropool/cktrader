#pragma once
#ifndef __CKTYPES_H__
#define __CKTYPES_H__

#include "ckdef.h"
#include "datablk.h"
#include <string>
#include <functional>
#include <memory>
#include <mutex>

struct CK_EXPORTS Task
{
	Task() :type(), task_data() {};
	Task(const Task& t) :type(t.type), task_data(t.task_data) {};

	std::string type;
	Datablk task_data;		//数据结构体
};

typedef std::function<void(Datablk&)> Handler;

struct CK_EXPORTS  FuncData
{
	FuncData() :h(), arg() {};
	FuncData(const FuncData& f) :h(f.h), arg(f.arg) {};

	Handler h; //事件处理函数
	Datablk arg;    //事件处理函数参数
};

//ctp
struct CK_EXPORTS CtpData
{
	CtpData() :task_name(0), task_data(), task_error(), task_id(0), task_last(true) {};
	CtpData(const CtpData& c) :task_name(c.task_name), 
						task_data(c.task_data),
						task_error(c.task_error),
						task_id(c.task_id),
						task_last(c.task_last) {};

	int task_name;		//回调函数名称对应的常量
	Datablk task_data;		//数据结构体
	Datablk task_error;		//错误结构体
	int task_id;		//请求id
	bool task_last;		//是否为最后返回
};

struct CK_EXPORTS StopOrder
{
	std::string symbol;
	std::string orderType;
	std::string direction;
	std::string offset;
	double price;
	int volume;
	std::string stopOrderID;
	std::string status;
	std::string strategyName;
};

struct CK_EXPORTS BarData
{
	std::string symbol;
	std::string exchange;
	double open;
	double high;
	double low;
	double close;
	std::string date;
	std::string time;
	int volume;
	int openInterest;
};

struct CK_EXPORTS TickData
{
	std::string symbol;
	std::string exchange;
	double lastPrice;
	int volume;
	int openInterest;
	double upperLimit;
	double lowerLimit;
	std::string date;
	std::string time;
	double bidPrice1;
	double bidPrice2;
	double bidPrice3;
	double bidPrice4;
	double bidPrice5;
	double askPrice1;
	double askPrice2;
	double askPrice3;
	double askPrice4;
	double askPrice5;
	int bidVolume1;
	int bidVolume2;
	int bidVolume3;
	int bidVolume4;
	int bidVolume5;
	int askVolume1;
	int askVolume2;
	int askVolume3;
	int askVolume4;
	int askVolume5;
	std::string tSymbol;
	std::string gateWayName;
	double openPrice;
	double highPrice;
	double lowPrice;
	double preClosePrice;
	int lastVolume;
};

struct CK_EXPORTS OrderData
{
	std::string symbol;
	std::string exchange;
	std::string orderID;
	std::string direction;
	std::string offset;
	double price;
	int totalVolume;
	int tradedVolume;
	std::string status;
	std::string orderTime;
	std::string cancelTime;
	int frontID;
	int sessionID;
	std::string tOrderID;
	std::string gateWayName;
	std::string tSymbol;
};

struct CK_EXPORTS TradeData
{
	std::string symbol;
	std::string exchange;
	std::string tradeID;
	std::string orderID;
	std::string direction;
	std::string offset;
	double price;
	int volume;
	std::string tradeTime;
	std::string tTradeID;
	std::string tOrderID;
	std::string gateWayName;
	std::string tSymbol;
};

struct CK_EXPORTS PositionData
{
	std::string symbol;
	std::string exchange;
	std::string tSymbol;
	std::string direction;
	int position;
	int frozen;
	double price;
	std::string tPositionName;
	int ydPosition;
	std::string gateWayName;
	int todayPosition;
	double todayPositionCost;
	double ydPositionCost;
};

struct CK_EXPORTS AccountData
{
	std::string accountID;
	std::string tAccountID;
	double preBalance;
	double balance;
	double available;
	double commission;
	double margin;
	double closeProfit;
	double positionProfit;
	std::string gateWayName;
};

struct CK_EXPORTS ErrorData
{
	std::string errorID;
	std::string errorMsg;
	std::string additionalInfo;
	std::string gateWayName;
};

struct CK_EXPORTS LogData
{
	std::string logContent;
	std::string gateWayName;
};

struct CK_EXPORTS ContractData
{
	std::string symbol;
	std::string exchange;
	std::string tSymbol;
	std::string name;
	std::string productClass;
	int size;
	double priceTick;
	double strikePrice;
	std::string underlyingSymbol;
	std::string optionType;
	std::string gateWayName;
};

struct CK_EXPORTS SubscribeReq
{
	std::string symbol;
	std::string exchange;
	std::string productClass;
	std::string currency;
	std::string expiry;
	double strikePrice;
	std::string optionType;
};

struct CK_EXPORTS OrderReq
{
	std::string symbol;
	std::string exchange;
	double price;
	int volume;
	std::string priceType;
	std::string direction;
	std::string offset;
	std::string productClass;
	std::string currency;
	std::string expiry;
	double strikePrice;
	std::string optionType;
	std::string strateyName;
};

struct CK_EXPORTS CancelOrderReq
{
	std::string symbol;
	std::string exchange;
	std::string orderID;
	std::string frontID;
	std::string sessionID;
};

#endif