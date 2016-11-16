#pragma once
#ifndef __CTPTD_H__
#define __CTPTD_H__

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"
#include "ThostFtdcTraderApi.h"
#include "utils/ckdef.h"
#include "utils/cktypes.h"

#include <map>
#include <mutex>
#include <string>

namespace cktrader {

	class EventEngine;
	class CTPGateWay;

	class CtpTd :public CThostFtdcTraderSpi
	{
	public:
		CtpTd(EventEngine *event_service, CTPGateWay *gateWay);
		~CtpTd();

		//-------------------------------------------------------------------------------------
		//API回调函数
		//-------------------------------------------------------------------------------------

		///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
		virtual void OnFrontConnected();

		///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
		///@param nReason 错误原因
		///        0x1001 网络读失败
		///        0x1002 网络写失败
		///        0x2001 接收心跳超时
		///        0x2002 发送心跳失败
		///        0x2003 收到错误报文
		virtual void OnFrontDisconnected(int nReason);

		///登录请求响应
		virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///登出请求响应
		virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///报单录入请求响应
		virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///报单操作请求响应
		virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///投资者结算结果确认响应
		virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///请求查询投资者持仓响应
		virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///请求查询资金账户响应
		virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///请求查询合约响应
		virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///错误应答
		virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///报单通知
		virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

		///成交通知
		virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

		///报单录入错误回报
		virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

		///报单操作错误回报
		virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);


		//-------------------------------------------------------------------------------------
		//task：任务
		//-------------------------------------------------------------------------------------
		void processFrontConnected(Datablk& data);

		void processFrontDisconnected(Datablk& data);


		void processRspUserLogin(Datablk& data);

		void processRspUserLogout(Datablk& data);

		void processRspOrderInsert(Datablk& data);

		void processRspOrderAction(Datablk& data);

		void processRspSettlementInfoConfirm(Datablk& data);

		void processRspQryInvestorPosition(Datablk& data);

		void processRspQryTradingAccount(Datablk& data);

		void processRspQryInstrument(Datablk& data);

		void processRspError(Datablk& data);

		void processRtnOrder(Datablk& data);

		void processRtnTrade(Datablk& data);

		void processErrRtnOrderInsert(Datablk& data);

		void processErrRtnOrderAction(Datablk& data);

		void connect(std::string userID, std::string password, std::string brokerID, std::string address);

		void login();
		void qryAccount();
		void qryPosition();
		std::string sendOrder(OrderReq& req);
		void cancelOrder(CancelOrderReq & req);
		void close();

	private:
		EventEngine *m_event_service;
		CTPGateWay *gateWay;
		std::string gateWayName;

		CThostFtdcTraderApi* api;

		int reqID = 0;//操作请求编号
		int orderRef = 0;// 订单编号

		bool connectionStatus = false;//      连接状态
		bool loginStatus = false;// 登录状态

		std::string userID;// 账号
		std::string password;//密码
		std::string brokerID;//经纪商代码
		std::string address;// 服务器地址

		int frontID;//前置机编号
		int sessionID;// 会话编号

		std::map<std::string, PositionData> *posBufferMap = nullptr;// 缓存持仓数据的字典

		mutable std::recursive_mutex the_mutex;
	};

}//cktrader

#endif // CTPTD_H