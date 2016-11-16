#pragma once
#ifndef __CTPMD_H__
#define __CTPMD_H__

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"
#include "ThostFtdcMdApi.h"
#include "utils/ckdef.h"
#include "utils/cktypes.h"

#include <set>
#include <string>
#include <mutex>

namespace cktrader {

	class EventEngine;
	class CTPGateWay;

	class CtpMd :public CThostFtdcMdSpi
	{
	public:
		CtpMd(EventEngine *event_service, CTPGateWay *gateWay);
		~CtpMd();

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

		///错误应答
		virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		///深度行情通知
		virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

		///事件处理函数

		void processFrontConnected(Datablk& data);

		void processFrontDisconnected(Datablk& data);

		void processRspUserLogin(Datablk& data);

		void processRspUserLogout(Datablk& data);

		void processRspError(Datablk& data);

		void processRtnDepthMarketData(Datablk& data);

		///////////////////////////////////////////////////////
		/// 主动调用函数
		///
		void connect(std::string userID, std::string password, std::string brokerID, std::string mdAddress);

		void subscribe(SubscribeReq& subscribeReq);

		int close();

		std::string getTradingDay();

		int subscribeMarketData(std::string instrumentID);

		int unSubscribeMarketData(std::string instrumentID);

		int subscribeForQuoteRsp(std::string instrumentID);

		int unSubscribeForQuoteRsp(std::string instrumentID);

	private:
		EventEngine *m_event_service;
		CTPGateWay *gateWay;
		CThostFtdcMdApi* api;

		std::string gateWayName;
		int reqID; //请求id

		bool connectionStatus = false;      // 连接状态
		bool loginStatus = false;//            登录状态

		std::set<std::string> *subscribedSymbols = nullptr;//    已订阅合约代码

		std::string userID;// 账号
		std::string password;//密码
		std::string brokerID;//        # 经纪商代码
		std::string address;//        # 服务器地址

		mutable std::recursive_mutex the_mutex;
	};

}//cktrader

#endif // CTPMD_H