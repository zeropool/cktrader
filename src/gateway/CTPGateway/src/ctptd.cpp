#include "utils/ckdef.h"
#include "utils/cktypes.h"
#include "ctptd.h"
#include "eventservice/eventengine.h"
#include "CTPGateway.h"

#include <sstream>
#include <direct.h>
#include <io.h>

namespace cktrader {
	CtpTd::CtpTd(EventEngine *event_service, CTPGateWay *gateWay)
	{
		this->m_event_service = event_service;
		this->gateWay = gateWay;
		this->gateWayName = gateWay->getName();
		posBufferMap = new std::map<std::string, PositionData>;

		reqID = 0;
		orderRef = 0;

		event_service->registerHandler(TDONFRONTCONNECTED, std::bind(&CtpTd::processFrontConnected, this, std::placeholders::_1));
		event_service->registerHandler(TDONFRONTDISCONNECTED, std::bind(&CtpTd::processFrontDisconnected, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPUSERLOGIN, std::bind(&CtpTd::processRspUserLogin, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPUSERLOGOUT, std::bind(&CtpTd::processRspUserLogout, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPORDERINSERT, std::bind(&CtpTd::processRspOrderInsert, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPORDERACTION, std::bind(&CtpTd::processRspOrderAction, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPSETTLEMENTINFOCONFIRM, std::bind(&CtpTd::processRspSettlementInfoConfirm, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPQRYINVESTORPOSITION, std::bind(&CtpTd::processRspQryInvestorPosition, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPQRYTRADINGACCOUNT, std::bind(&CtpTd::processRspQryTradingAccount, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPQRYINSTRUMENT, std::bind(&CtpTd::processRspQryInstrument, this, std::placeholders::_1));
		event_service->registerHandler(TDONRSPERROR, std::bind(&CtpTd::processRspError, this, std::placeholders::_1));
		event_service->registerHandler(TDONRTNORDER, std::bind(&CtpTd::processRtnOrder, this, std::placeholders::_1));
		event_service->registerHandler(TDONRTNTRADE, std::bind(&CtpTd::processRtnTrade, this, std::placeholders::_1));
		event_service->registerHandler(TDONERRRTNORDERINSERT, std::bind(&CtpTd::processErrRtnOrderInsert, this, std::placeholders::_1));
		event_service->registerHandler(TDONERRRTNORDERACTION, std::bind(&CtpTd::processErrRtnOrderAction, this, std::placeholders::_1));
	}

	CtpTd::~CtpTd()
	{
		delete posBufferMap;
		posBufferMap = nullptr;
	}

	//-------------------------------------------------------------------------------------
	//API回调函数
	//-------------------------------------------------------------------------------------

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void CtpTd::OnFrontConnected()
	{
		Task task = Task();
		task.type = TDONFRONTCONNECTED;
		m_event_service->put(task);
	}

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void CtpTd::OnFrontDisconnected(int nReason)
	{
		CtpData data = CtpData();
		data.task_id = nReason;

		Task task = Task();
		task.type = TDONFRONTDISCONNECTED;
		task.task_data = data;
		m_event_service->put(task);
	}

	///登录请求响应
	void CtpTd::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pRspUserLogin)
		{
			data.task_data = *pRspUserLogin;
		}
		else
		{
			CThostFtdcRspUserLoginField empty_data = CThostFtdcRspUserLoginField();
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
		task.type = TDONRSPUSERLOGIN;
		task.task_data = data;
		m_event_service->put(task);
	}

	///登出请求响应
	void CtpTd::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
		task.type = TDONRSPUSERLOGOUT;
		task.task_data = data;
		m_event_service->put(task);
	}

	///报单录入请求响应
	void CtpTd::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pInputOrder)
		{
			data.task_data = *pInputOrder;
		}
		else
		{
			CThostFtdcInputOrderField empty_data = CThostFtdcInputOrderField();
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
		task.type = TDONRSPORDERINSERT;
		task.task_data = data;
		m_event_service->put(task);
	}

	///报单操作请求响应
	void CtpTd::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pInputOrderAction)
		{
			data.task_data = *pInputOrderAction;
		}
		else
		{
			CThostFtdcInputOrderActionField empty_data = CThostFtdcInputOrderActionField();
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
		task.type = TDONRSPORDERACTION;
		task.task_data = data;
		m_event_service->put(task);
	}

	///投资者结算结果确认响应
	void CtpTd::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pSettlementInfoConfirm)
		{
			data.task_data = *pSettlementInfoConfirm;
		}
		else
		{
			CThostFtdcSettlementInfoConfirmField empty_data = CThostFtdcSettlementInfoConfirmField();
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
		task.type = TDONRSPSETTLEMENTINFOCONFIRM;
		task.task_data = data;
		m_event_service->put(task);
	}

	///请求查询投资者持仓响应
	void CtpTd::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pInvestorPosition)
		{
			data.task_data = *pInvestorPosition;
		}
		else
		{
			CThostFtdcInvestorPositionField empty_data = CThostFtdcInvestorPositionField();
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
		task.type = TDONRSPQRYINVESTORPOSITION;
		task.task_data = data;
		m_event_service->put(task);
	}

	///请求查询资金账户响应
	void CtpTd::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pTradingAccount)
		{
			data.task_data = *pTradingAccount;
		}
		else
		{
			CThostFtdcTradingAccountField empty_data = CThostFtdcTradingAccountField();
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
		task.type = TDONRSPQRYTRADINGACCOUNT;
		task.task_data = data;
		m_event_service->put(task);
	}

	///请求查询合约响应
	void CtpTd::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		CtpData data = CtpData();

		if (pInstrument)
		{
			data.task_data = *pInstrument;
		}
		else
		{
			CThostFtdcInstrumentField empty_data = CThostFtdcInstrumentField();
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
		task.type = TDONRSPQRYINSTRUMENT;
		task.task_data = data;
		m_event_service->put(task);
	}

	///错误应答
	void CtpTd::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
		task.type = TDONRSPERROR;
		task.task_data = data;
		m_event_service->put(task);
	}

	///报单通知
	void CtpTd::OnRtnOrder(CThostFtdcOrderField *pOrder)
	{
		CtpData data = CtpData();

		if (pOrder)
		{
			data.task_data = *pOrder;
		}
		else
		{
			CThostFtdcOrderField empty_data = CThostFtdcOrderField();
			memset(&empty_data, 0, sizeof(empty_data));
			data.task_data = empty_data;
		}

		Task task = Task();
		task.type = TDONRTNORDER;
		task.task_data = data;
		m_event_service->put(task);
	}

	///成交通知
	void CtpTd::OnRtnTrade(CThostFtdcTradeField *pTrade)
	{
		CtpData data = CtpData();

		if (pTrade)
		{
			data.task_data = *pTrade;
		}
		else
		{
			CThostFtdcTradeField empty_data = CThostFtdcTradeField();
			memset(&empty_data, 0, sizeof(empty_data));
			data.task_data = empty_data;
		}

		Task task = Task();
		task.type = TDONRTNTRADE;
		task.task_data = data;
		m_event_service->put(task);
	}

	///报单录入错误回报
	void CtpTd::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
	{
		CtpData data = CtpData();

		if (pInputOrder)
		{
			data.task_data = *pInputOrder;
		}
		else
		{
			CThostFtdcInputOrderField empty_data = CThostFtdcInputOrderField();
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

		Task task = Task();
		task.type = TDONERRRTNORDERINSERT;
		task.task_data = data;
		m_event_service->put(task);
	}

	///报单操作错误回报
	void CtpTd::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
	{
		CtpData data = CtpData();

		if (pOrderAction)
		{
			data.task_data = *pOrderAction;
		}
		else
		{
			CThostFtdcOrderActionField empty_data = CThostFtdcOrderActionField();
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

		Task task = Task();
		task.type = TDONERRRTNORDERACTION;
		task.task_data = data;
		m_event_service->put(task);
	}


	//-------------------------------------------------------------------------------------
	//task：任务
	//-------------------------------------------------------------------------------------
	void CtpTd::processFrontConnected(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		connectionStatus = true;

		LogData log = LogData();
		log.gateWayName = gateWayName;
		log.logContent = "交易服务器连接成功";
		gateWay->onLog(log);

		login();
	}

	void CtpTd::processFrontDisconnected(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		connectionStatus = false;
		loginStatus = false;
		gateWay->tdConnected = false;

		LogData log = LogData();
		log.gateWayName = gateWayName;
		log.logContent = "交易服务器连接断开";
		gateWay->onLog(log);
	}


	void CtpTd::processRspUserLogin(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspUserLoginField task_data = ctp_data.task_data.cast<CThostFtdcRspUserLoginField>();

		CThostFtdcRspInfoField task_error = ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		if (task_error.ErrorID == 0)
		{
			frontID = task_data.FrontID;
			sessionID = task_data.SessionID;
			loginStatus = true;
			gateWay->tdConnected = true;

			LogData log = LogData();
			log.gateWayName = gateWayName;
			log.logContent = "交易服务器登录完成";
			gateWay->onLog(log);

			CThostFtdcSettlementInfoConfirmField myreq = CThostFtdcSettlementInfoConfirmField();
			memset(&myreq, 0, sizeof(myreq));
			strncpy(myreq.InvestorID, userID.c_str(), sizeof(myreq.InvestorID) - 1);
			strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
			reqID++;
			api->ReqSettlementInfoConfirm(&myreq, reqID);
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

	void CtpTd::processRspUserLogout(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		if (task_error.ErrorID == 0)
		{
			loginStatus = false;
			gateWay->tdConnected = false;

			LogData log = LogData();
			log.gateWayName = gateWayName;
			log.logContent = "交易服务器登出完成";
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

	void CtpTd::processRspOrderInsert(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error =	ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		ErrorData err = ErrorData();
		err.gateWayName = gateWayName;
		err.errorID = task_error.ErrorID;
		err.errorMsg = task_error.ErrorMsg;
		gateWay->onError(err);
	}

	void CtpTd::processRspOrderAction(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		ErrorData err = ErrorData();
		err.gateWayName = gateWayName;
		err.errorID = task_error.ErrorID;
		err.errorMsg = task_error.ErrorMsg;
		gateWay->onError(err);
	}

	void CtpTd::processRspSettlementInfoConfirm(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		LogData log = LogData();
		log.gateWayName = gateWayName;
		log.logContent = "结算信息确认完成";
		gateWay->onLog(log);

		CThostFtdcQryInstrumentField myreq = CThostFtdcQryInstrumentField();
		memset(&myreq, 0, sizeof(myreq));
		reqID++;
		api->ReqQryInstrument(&myreq, reqID);
	}

	void CtpTd::processRspQryInvestorPosition(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcInvestorPositionField task_data = ctp_data.task_data.cast<CThostFtdcInvestorPositionField>();

		std::string direction;
		std::stringstream stream;
		stream << task_data.PosiDirection;
		direction = stream.str();
		std::string positionName = std::string(task_data.InstrumentID) + "." + direction;

		PositionData posData = PositionData();


		if (posBufferMap->find(positionName) != posBufferMap->end())
		{
			posData = posBufferMap->at(positionName);
		}
		else
		{
			posData.symbol = task_data.InstrumentID;
			posData.tSymbol = task_data.InstrumentID;
			posData.direction = direction;
			posData.frozen = task_data.FrozenCash;
			posData.tPositionName = positionName;
			posData.gateWayName = gateWayName;
		}

		if (task_data.TodayPosition)
		{
			posData.todayPosition = task_data.Position;
			posData.todayPositionCost = task_data.PositionCost;
		}
		else
		{
			if (task_data.YdPosition)
			{
				posData.ydPosition = task_data.Position;
				posData.ydPositionCost = task_data.PositionCost;
			}
		}

		posData.position = posData.todayPosition + posData.ydPosition;

		if (posData.todayPosition || posData.ydPosition)
		{
			posData.price = (posData.ydPositionCost + posData.todayPositionCost) / (posData.todayPosition + posData.ydPosition);
		}
		else
		{
			posData.price = 0;
		}

		gateWay->onPosition(posData);

		posBufferMap->insert(std::pair<std::string, PositionData>(positionName, posData));
	}

	void CtpTd::processRspQryTradingAccount(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcTradingAccountField task_data = ctp_data.task_data.cast<CThostFtdcTradingAccountField>();

		AccountData account = AccountData();
		account.gateWayName = gateWayName;

		account.accountID = task_data.AccountID;
		account.tAccountID = std::string(gateWayName) + "." + std::string(account.accountID);

		account.preBalance = task_data.PreBalance;
		account.available = task_data.Available;
		account.commission = task_data.Commission;
		account.margin = task_data.CurrMargin;
		account.closeProfit = task_data.CloseProfit;
		account.positionProfit = task_data.PositionProfit;

		account.balance = task_data.PreBalance - task_data.PreCredit - task_data.PreMortgage + \
			task_data.Mortgage - task_data.Withdraw + task_data.Deposit + \
			task_data.CloseProfit + task_data.PositionProfit + task_data.CashIn - \
			task_data.Commission;

		gateWay->onAccount(account);
	}

	void CtpTd::processRspQryInstrument(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcInstrumentField task_data = ctp_data.task_data.cast<CThostFtdcInstrumentField>();

		ContractData contract = ContractData();
		contract.gateWayName = gateWayName;

		contract.symbol = task_data.InstrumentID;
		contract.exchange = task_data.ExchangeID;
		contract.tSymbol = contract.symbol;
		contract.name = task_data.InstrumentName;

		contract.size = task_data.VolumeMultiple;
		contract.priceTick = task_data.PriceTick;
		contract.strikePrice = task_data.StrikePrice;
		contract.underlyingSymbol = task_data.UnderlyingInstrID;

		if (task_data.ProductClass == '1')
		{
			contract.productClass = PRODUCT_FUTURES;
		}
		else if (task_data.ProductClass == '2')
		{
			contract.productClass = PRODUCT_OPTION;
		}
		else if (task_data.ProductClass == '3')
		{
			contract.productClass = PRODUCT_COMBINATION;
		}
		else
		{
			contract.productClass = PRODUCT_UNKNOWN;
		}

		if (task_data.OptionsType == '1')
		{
			contract.optionType = OPTION_CALL;
		}
		else if (task_data.OptionsType == '2')
		{
			contract.optionType = OPTION_PUT;
		}

		gateWay->onContract(contract);

		if (ctp_data.task_last)
		{
			LogData log = LogData();
			log.gateWayName = gateWayName;
			log.logContent = "交易合约信息获取完成";
			gateWay->onLog(log);
		}
	}

	void CtpTd::processRspError(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		ErrorData err = ErrorData();
		err.gateWayName = gateWayName;
		err.errorID = task_error.ErrorID;
		err.errorMsg = task_error.ErrorMsg;
		gateWay->onError(err);
	}

	void CtpTd::processRtnOrder(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcOrderField task_data = ctp_data.task_data.cast<CThostFtdcOrderField>();

		int newref = std::stoi(std::string(task_data.OrderRef), nullptr);
		orderRef = std::max(orderRef, newref);

		OrderData order = OrderData();
		order.gateWayName = gateWayName;

		order.symbol = task_data.InstrumentID;
		order.exchange = task_data.ExchangeID;
		order.tSymbol = order.symbol;

		order.orderID = std::string(task_data.OrderRef);

		if (task_data.Direction == '0')
		{
			order.direction = DIRECTION_LONG;
		}
		else if (task_data.Direction == '1')
		{
			order.direction = DIRECTION_SHORT;
		}
		else
		{
			order.direction = DIRECTION_UNKNOWN;
		}

		if (task_data.CombOffsetFlag[0] == '0')
		{
			order.offset = OFFSET_OPEN;
		}
		else if (task_data.CombOffsetFlag[0] == '1')
		{
			order.offset = OFFSET_CLOSE;
		}
		else
		{
			order.offset = OFFSET_UNKNOWN;
		}

		if (task_data.OrderStatus == '0')
		{
			order.status = STATUS_ALLTRADED;
		}
		else if (task_data.OrderStatus == '1')
		{
			order.status = STATUS_PARTTRADED;
		}
		else if (task_data.OrderStatus == '3')
		{
			order.status = STATUS_NOTTRADED;
		}
		else if (task_data.OrderStatus == '5')
		{
			order.status = STATUS_CANCELLED;
		}
		else
		{
			order.status = STATUS_UNKNOWN;
		}


		order.price = task_data.LimitPrice;
		order.totalVolume = task_data.VolumeTotalOriginal;
		order.tradedVolume = task_data.VolumeTraded;
		order.orderTime = task_data.InsertTime;
		order.cancelTime = task_data.CancelTime;
		order.frontID = task_data.FrontID;
		order.sessionID = task_data.SessionID;

		//CTP的报单号一致性维护需要基于frontID, sessionID, orderID三个字段
		//但在本接口设计中，已经考虑了CTP的OrderRef的自增性，避免重复
		//唯一可能出现OrderRef重复的情况是多处登录并在非常接近的时间内（几乎同时发单）
		// 考虑到VtTrader的应用场景，认为以上情况不会构成问题
		order.tOrderID = gateWayName + '.' + order.orderID;
		gateWay->onOrder(order);
	}

	void CtpTd::processRtnTrade(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcTradeField task_data = ctp_data.task_data.cast<CThostFtdcTradeField>();

		TradeData trade = TradeData();
		trade.gateWayName = gateWayName;

		//保存代码和报单号
		trade.symbol = task_data.InstrumentID;
		trade.exchange = task_data.ExchangeID;
		trade.tSymbol = trade.symbol;

		trade.tradeID = task_data.TradeID;
		trade.tTradeID = gateWayName + "." + std::string(trade.tradeID);

		trade.orderID = task_data.OrderRef;
		trade.tOrderID = gateWayName + "." + std::string(trade.orderID);

		// 方向
		if (task_data.Direction == THOST_FTDC_D_Buy)
		{
			trade.direction = DIRECTION_LONG;
		}
		else if (task_data.Direction == THOST_FTDC_D_Sell)
		{
			trade.direction = DIRECTION_SHORT;
		}

		//开平
		if (task_data.OffsetFlag == THOST_FTDC_OF_Open)
		{
			trade.offset = OFFSET_OPEN;
		}
		else if (task_data.OffsetFlag == THOST_FTDC_OF_Close)
		{
			trade.offset = OFFSET_CLOSE;
		}
		else if (task_data.OffsetFlag == THOST_FTDC_OF_CloseToday)
		{
			trade.offset = OFFSET_CLOSETODAY;
		}
		else if (task_data.OffsetFlag == THOST_FTDC_OF_CloseYesterday)
		{
			trade.offset = OFFSET_CLOSEYESTERDAY;
		}

		//价格、报单量等数值
		trade.price = task_data.Price;
		trade.volume = task_data.Volume;
		trade.tradeTime = task_data.TradeTime;

		gateWay->onTrade(trade);
	}

	void CtpTd::processErrRtnOrderInsert(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		ErrorData err = ErrorData();
		err.gateWayName = gateWayName;
		err.errorID = task_error.ErrorID;
		err.errorMsg = task_error.ErrorMsg;
		gateWay->onError(err);
	}

	void CtpTd::processErrRtnOrderAction(Datablk& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		CtpData ctp_data = data.cast<CtpData>();

		CThostFtdcRspInfoField task_error = ctp_data.task_error.cast<CThostFtdcRspInfoField>();

		ErrorData err = ErrorData();
		err.gateWayName = gateWayName;
		err.errorID = task_error.ErrorID;
		err.errorMsg = task_error.ErrorMsg;
		gateWay->onError(err);
	}

	void CtpTd::connect(std::string userID, std::string password, std::string brokerID, std::string address)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		this->userID = userID;
		this->password = password;
		this->brokerID = brokerID;
		this->address = address;

		if (!connectionStatus)
		{
			char dir_ptr[512];
			std::string pwd = getcwd(dir_ptr, 512);
			pwd = pwd + CTP_PATH_COM;

			if (access(pwd.c_str(), 0) == -1)
			{
				mkdir(pwd.c_str());
			}

			api = CThostFtdcTraderApi::CreateFtdcTraderApi(pwd.c_str());
			api->RegisterSpi(this);
			api->RegisterFront((char*)address.c_str());
			api->SubscribePublicTopic(THOST_TERT_QUICK);
			api->SubscribePrivateTopic(THOST_TERT_QUICK);
			api->Init();
		}
		else if (!loginStatus)
		{
			login();
		}
	}

	void CtpTd::login()
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		reqID++;
		CThostFtdcReqUserLoginField myreq = CThostFtdcReqUserLoginField();
		memset(&myreq, 0, sizeof(myreq));

		if (userID.size() != 0 && password.size() != 0 && brokerID.size() != 0)
		{
			strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
			strncpy(myreq.UserID, userID.c_str(), sizeof(myreq.UserID) - 1);
			strncpy(myreq.Password, password.c_str(), sizeof(myreq.Password) - 1);
		}

		api->ReqUserLogin(&myreq, reqID);
	}

	void CtpTd::qryAccount()
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		reqID++;
		CThostFtdcQryTradingAccountField myreq = CThostFtdcQryTradingAccountField();
		memset(&myreq, 0, sizeof(myreq));

		strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.InvestorID, userID.c_str(), sizeof(myreq.InvestorID) - 1);

		this->api->ReqQryTradingAccount(&myreq, reqID);
	}

	void CtpTd::qryPosition()
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		reqID++;

		CThostFtdcQryInvestorPositionField myreq = CThostFtdcQryInvestorPositionField();
		memset(&myreq, 0, sizeof(myreq));
		strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.InvestorID, userID.c_str(), sizeof(myreq.InvestorID) - 1);
		this->api->ReqQryInvestorPosition(&myreq, reqID);
	}

	std::string CtpTd::sendOrder(OrderReq& req)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		reqID++;

		orderRef++;

		CThostFtdcInputOrderField myreq = CThostFtdcInputOrderField();
		memset(&myreq, 0, sizeof(myreq));

		strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.InvestorID, userID.c_str(), sizeof(myreq.InvestorID) - 1);
		strncpy(myreq.OrderRef, std::to_string(orderRef).c_str(), sizeof(myreq.OrderRef) - 1);
		strncpy(myreq.InstrumentID, req.symbol.c_str(), sizeof(myreq.InstrumentID) - 1);
		strncpy(myreq.UserID, userID.c_str(), sizeof(myreq.UserID) - 1);

		myreq.LimitPrice = req.price;
		myreq.VolumeTotalOriginal = req.volume;
		if (req.priceType == PRICETYPE_LIMITPRICE)
		{
			myreq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		}
		if (req.priceType == PRICETYPE_MARKETPRICE)
		{
			myreq.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
		}

		if (req.direction == DIRECTION_LONG)
		{
			myreq.Direction = THOST_FTDC_D_Buy;
		}
		if (req.direction == DIRECTION_SHORT)
		{
			myreq.Direction = THOST_FTDC_D_Sell;
		}

		if (req.offset == OFFSET_OPEN)
		{
			myreq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
		}
		if (req.offset == OFFSET_CLOSE)
		{
			myreq.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
		}
		if (req.offset == OFFSET_CLOSETODAY)
		{
			myreq.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
		}
		if (req.offset == OFFSET_CLOSEYESTERDAY)
		{
			myreq.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
		}

		myreq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
		myreq.ContingentCondition = THOST_FTDC_CC_Immediately;
		myreq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		myreq.IsAutoSuspend = 0;
		myreq.TimeCondition = THOST_FTDC_TC_GFD;
		myreq.VolumeCondition = THOST_FTDC_VC_AV;
		myreq.MinVolume = 1;

		if (req.priceType == PRICETYPE_FAK)
		{
			myreq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			myreq.TimeCondition = THOST_FTDC_TC_IOC;
			myreq.VolumeCondition = THOST_FTDC_VC_AV;
		}
		if (req.priceType == PRICETYPE_FOK)
		{
			myreq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			myreq.TimeCondition = THOST_FTDC_TC_IOC;
			myreq.VolumeCondition = THOST_FTDC_VC_CV;
		}

		this->api->ReqOrderInsert(&myreq, reqID);

		return std::to_string(orderRef);
	}

	void CtpTd::cancelOrder(CancelOrderReq & req)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		reqID++;

		CThostFtdcInputOrderActionField myreq = CThostFtdcInputOrderActionField();
		memset(&myreq, 0, sizeof(myreq));

		strncpy(myreq.BrokerID, brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.InvestorID, userID.c_str(), sizeof(myreq.InvestorID) - 1);
		strncpy(myreq.InstrumentID, req.symbol.c_str(), sizeof(myreq.InstrumentID) - 1);
		strncpy(myreq.ExchangeID, req.exchange.c_str(), sizeof(myreq.ExchangeID) - 1);

		strncpy(myreq.OrderRef, req.orderID.c_str(), sizeof(myreq.OrderRef) - 1);
		myreq.FrontID = frontID;
		myreq.SessionID = sessionID;

		myreq.ActionFlag = THOST_FTDC_AF_Delete;

		this->api->ReqOrderAction(&myreq, reqID);
	}

	void CtpTd::close()
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		this->api->RegisterSpi(NULL);
		this->api->Release();
		this->api = nullptr;
	}

}//cktrader