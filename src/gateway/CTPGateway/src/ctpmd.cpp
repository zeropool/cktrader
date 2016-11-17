#include "utils/ckdef.h"
#include "utils/cktypes.h"
#include "CTPGateway.h"
#include "ctpmd.h"
#include "eventservice/eventengine.h"

#include <direct.h>
#include <io.h>

namespace cktrader {

	CtpMd::CtpMd(EventEngine *event_service, CTPGateWay *gateWay)
	{
		this->m_event_service = event_service;
		this->gateWay = gateWay;
		this->gateWayName = gateWay->getName();
		subscribedSymbols = new std::set<std::string>;

		//register event handler
		m_event_service->registerHandler(MDONFRONTCONNECTED, std::bind(&CtpMd::processFrontConnected, this, std::placeholders::_1));
		m_event_service->registerHandler(MDONFRONTDISCONNECTED, std::bind(&CtpMd::processFrontDisconnected, this, std::placeholders::_1));
		m_event_service->registerHandler(MDONRSPUSERLOGIN, std::bind(&CtpMd::processRspUserLogin, this, std::placeholders::_1));
		m_event_service->registerHandler(MDONRSPUSERLOGOUT, std::bind(&CtpMd::processRspUserLogout, this, std::placeholders::_1));
		m_event_service->registerHandler(MDONRSPERROR, std::bind(&CtpMd::processRspError, this, std::placeholders::_1));
		m_event_service->registerHandler(MDONRTNDEPTHMARKETDATA, std::bind(&CtpMd::processRtnDepthMarketData, this, std::placeholders::_1));
	}

	CtpMd::~CtpMd()
	{
		delete subscribedSymbols;
		subscribedSymbols = nullptr;
	}

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void CtpMd::OnFrontConnected()
	{
		CtpData data = CtpData();

		Task task = Task();
		task.type = MDONFRONTCONNECTED;
		task.task_data = data;
		m_event_service->put(task);
	}

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void CtpMd::OnFrontDisconnected(int nReason)
	{
		CtpData data = CtpData();
		data.task_id = nReason;

		Task task = Task();
		task.type = MDONFRONTDISCONNECTED;
		task.task_data = data;
		m_event_service->put(task);
	}

	///登录请求响应
	void CtpMd::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pRspUserLogin)
		{
			data.task_data = *pRspUserLogin;
		}
		else
		{
			CThostFtdcRspUserLoginField empty_data = CThostFtdcRspUserLoginField();
			memset(&empty_data, 0, sizeof(CThostFtdcRspUserLoginField));
			data.task_data = empty_data;
		}

		if (pRspInfo)
		{
			data.task_error = *pRspInfo;
		}
		else
		{
			CThostFtdcRspInfoField empty_error = CThostFtdcRspInfoField();
			memset(&empty_error, 0, sizeof(empty_error));
			data.task_error = empty_error;
		}
		data.task_id = nRequestID;
		data.task_last = bIsLast;

		Task task = Task();
		task.type = MDONRSPUSERLOGIN;
		task.task_data = data;
		m_event_service->put(task);
	}

	///登出请求响应
	void CtpMd::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pUserLogout)
		{
			data.task_data = *pUserLogout;
		}
		else
		{
			CThostFtdcUserLogoutField empty_data = CThostFtdcUserLogoutField();
			memset(&empty_data, 0, sizeof(empty_data));
			data.task_data = empty_data;
		}

		if (pRspInfo)
		{
			data.task_error = *pRspInfo;
		}
		else
		{
			CThostFtdcRspInfoField empty_error = CThostFtdcRspInfoField();
			memset(&empty_error, 0, sizeof(empty_error));
			data.task_error = empty_error;
		}
		data.task_id = nRequestID;
		data.task_last = bIsLast;

		Task task = Task();
		task.type = MDONRSPUSERLOGOUT;
		task.task_data = data;
		m_event_service->put(task);
	}

	///错误应答
	void CtpMd::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pRspInfo)
		{
			data.task_error = *pRspInfo;
		}
		else
		{
			CThostFtdcRspInfoField empty_error = CThostFtdcRspInfoField();
			memset(&empty_error, 0, sizeof(empty_error));
			data.task_error = empty_error;
		}
		data.task_id = nRequestID;
		data.task_last = bIsLast;

		Task task = Task();
		task.type = MDONRSPERROR;
		task.task_data = data;
		m_event_service->put(task);
	}

	///深度行情通知
	void CtpMd::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
	{
		CtpData data = CtpData();

		if (pDepthMarketData)
		{
			data.task_data = *pDepthMarketData;
		}
		else
		{
			CThostFtdcDepthMarketDataField empty_data = CThostFtdcDepthMarketDataField();
			memset(&empty_data, 0, sizeof(empty_data));
			data.task_data = empty_data;
		}

		Task task = Task();
		task.type = MDONRTNDEPTHMARKETDATA;
		task.task_data = data;
		m_event_service->put(task);
	}

	///事件处理函数

	void CtpMd::processFrontConnected(Datablk& data)
	{
		connectionStatus = true;

		CtpData ctpdata = data.cast<CtpData>();

		LogData log = LogData();
		log.gateWayName = gateWayName;
		log.logContent = "行情服务器连接成功";
		gateWay->onLog(log);

		reqID++;
		CThostFtdcReqUserLoginField myreq = CThostFtdcReqUserLoginField();
		memset(&myreq, 0, sizeof(myreq));

		if (userID.length() != 0 && password.length() != 0 && brokerID.length() != 0)
		{
			strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
			strncpy(myreq.UserID, userID.c_str(), sizeof(myreq.UserID) - 1);
			strncpy(myreq.Password, password.c_str(), sizeof(myreq.Password) - 1);
		}

		api->ReqUserLogin(&myreq, reqID);
	}

	void CtpMd::processFrontDisconnected(Datablk& data)
	{
		connectionStatus = false;
		loginStatus = false;
		gateWay->mdConnected = false;

		CtpData ctpdata = data.cast<CtpData>();

		LogData log = LogData();
		log.gateWayName = gateWayName;
		log.logContent = "行情服务器连接断开";
		gateWay->onLog(log);
	}

	void CtpMd::processRspUserLogin(Datablk& data)
	{
		CtpData ctpdata = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctpdata.task_error.cast<CThostFtdcRspInfoField>();

		if (task_error.ErrorID == 0)
		{
			loginStatus = true;
			gateWay->mdConnected = true;
			LogData log = LogData();
			log.gateWayName = gateWayName;
			log.logContent = "行情服务器登录完成";
			gateWay->onLog(log);

			for (std::set<std::string>::iterator it = subscribedSymbols->begin(); it != subscribedSymbols->end(); it++)
			{
				subscribeMarketData(*it);
			}
		}
		else
		{
			ErrorData err = ErrorData();
			err.gateWayName = gateWayName;
			err.errorID = task_error.ErrorID;
			err.errorMsg = task_error.ErrorMsg;
			gateWay->onError(err);
		}
	}

	void CtpMd::processRspUserLogout(Datablk& data)
	{
		CtpData ctpdata = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctpdata.task_error.cast<CThostFtdcRspInfoField>();

		if (task_error.ErrorID == 0)
		{
			loginStatus = false;
			gateWay->mdConnected = false;
			LogData log = LogData();
			log.gateWayName = gateWayName;
			log.logContent = "行情服务器登出完成";
			gateWay->onLog(log);
		}
		else
		{
			ErrorData err = ErrorData();
			err.gateWayName = gateWayName;
			err.errorID = task_error.ErrorID;
			err.errorMsg = task_error.ErrorMsg;
			gateWay->onError(err);
		}
	}

	void CtpMd::processRspError(Datablk& data)
	{
		CtpData ctpdata = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctpdata.task_error.cast<CThostFtdcRspInfoField>();

		ErrorData err = ErrorData();
		err.gateWayName = gateWayName;
		err.errorID = task_error.ErrorID;
		err.errorMsg = task_error.ErrorMsg;
		gateWay->onError(err);
	}

	void CtpMd::processRtnDepthMarketData(Datablk& data)
	{
		CtpData ctpdata = data.cast<CtpData>();

		CThostFtdcDepthMarketDataField task_data = ctpdata.task_data.cast<CThostFtdcDepthMarketDataField>();

		TickData tick;

		tick.gateWayName = gateWayName;

		tick.symbol = task_data.InstrumentID;
		tick.exchange = task_data.ExchangeID;
		tick.tSymbol = tick.symbol;
		tick.lastPrice = task_data.LastPrice;
		tick.volume = task_data.Volume;
		tick.openInterest = task_data.OpenInterest;
		tick.time = std::string(task_data.UpdateTime) + "." + std::to_string(task_data.UpdateMillisec / 100);
		tick.date = task_data.TradingDay;

		tick.openPrice = task_data.OpenPrice;
		tick.highPrice = task_data.HighestPrice;
		tick.lowPrice = task_data.LowestPrice;
		tick.preClosePrice = task_data.PreClosePrice;

		tick.upperLimit = task_data.UpperLimitPrice;
		tick.lowerLimit = task_data.LowerLimitPrice;

		//CTP只有一档行情
		tick.bidPrice1 = task_data.BidPrice1;
		tick.bidVolume1 = task_data.BidVolume1;
		tick.askPrice1 = task_data.AskPrice1;
		tick.askVolume1 = task_data.AskVolume1;

		gateWay->onTick(tick);
	}

	///////////////////////////////////////////////////////
	/// 主动调用函数
	///
	void CtpMd::connect(std::string userID, std::string password, std::string brokerID, std::string mdAddress)
	{
		this->userID = userID;
		this->password = password;
		this->brokerID = brokerID;
		this->address = mdAddress;

		if (!connectionStatus)
		{
			char dir_ptr[512];
			std::string pwd = getcwd(dir_ptr, 512);
			pwd = pwd + CTP_PATH_COM;

			if (access(pwd.c_str(), 0) == -1)
			{
				mkdir(pwd.c_str());
			}
			this->api = CThostFtdcMdApi::CreateFtdcMdApi(pwd.c_str());
			this->api->RegisterSpi(this);

			this->api->RegisterFront((char*)mdAddress.c_str());
			this->api->Init();
		}
		else
		{
			if (!loginStatus)
			{
				reqID++;
				CThostFtdcReqUserLoginField myreq = CThostFtdcReqUserLoginField();

				if (userID.length() != 0 && password.length() != 0 && brokerID.length() != 0)
				{
					strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
					strncpy(myreq.UserID, userID.c_str(), sizeof(myreq.UserID) - 1);
					strncpy(myreq.Password, password.c_str(), sizeof(myreq.Password) - 1);
				}

				api->ReqUserLogin(&myreq, reqID);
			}
		}
	}

	void CtpMd::subscribe(SubscribeReq& subscribeReq)
	{
		if (loginStatus)
		{
			subscribeMarketData(subscribeReq.symbol);
		}

		subscribedSymbols->insert(subscribeReq.symbol);
	}

	int CtpMd::close()
	{
		//该函数在原生API里没有，用于安全退出API用，原生的join似乎不太稳定
		this->api->RegisterSpi(NULL);
		this->api->Release();
		this->api = NULL;
		return 1;
	}

	std::string CtpMd::getTradingDay()
	{
		return api->GetTradingDay();
	}

	int CtpMd::subscribeMarketData(std::string instrumentID)
	{
		char* buffer = (char*)instrumentID.c_str();
		char* myreq[1] = { buffer };

		int i = this->api->SubscribeMarketData(myreq, 1);
		return i;
	}

	int CtpMd::unSubscribeMarketData(std::string instrumentID)
	{
		char* buffer = (char*)instrumentID.c_str();
		char* myreq[1] = { buffer };

		int i = this->api->UnSubscribeMarketData(myreq, 1);
		return i;
	}

	int CtpMd::subscribeForQuoteRsp(std::string instrumentID)
	{
		char* buffer = (char*)instrumentID.c_str();
		char* myreq[1] = { buffer };
		int i = this->api->SubscribeForQuoteRsp(myreq, 1);
		return i;
	}

	int CtpMd::unSubscribeForQuoteRsp(std::string instrumentID)
	{
		char* buffer = (char*)instrumentID.c_str();
		char* myreq[1] = { buffer };;
		int i = this->api->UnSubscribeForQuoteRsp(myreq, 1);
		return i;
	}

}//cktrader