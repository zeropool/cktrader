#include "servicemgr_iml.h"
#include "utils/json11.hpp"

#include <fstream> 
#include <sstream>

namespace cktrader {

ServiceMgr::ServiceMgr():the_mutex()
{
	m_DLLMap = new std::map<std::string, CDllHelper*>;//装载动态加载的dll

	m_GateWayMap = new std::map<std::string, IGateway*>;//装载gateway

	m_StrategyMap = new std::map<std::string, IStrategy*>;//装载strategy

	initGateway();
	initStrategy();
}

ServiceMgr::ServiceMgr(ServiceMgr& mgr):the_mutex()
{
	for (std::map<std::string, CDllHelper*>::iterator it = mgr.m_DLLMap->begin(); it != mgr.m_DLLMap->end(); ++it)
	{
		m_DLLMap->insert(std::make_pair(it->first, it->second));
	}

	for (std::map<std::string, IGateway*>::iterator it = mgr.m_GateWayMap->begin(); it != mgr.m_GateWayMap->end(); ++it)
	{
		m_GateWayMap->insert(std::make_pair(it->first, it->second));
	}

	for (std::map<std::string, IStrategy*>::iterator it = mgr.m_StrategyMap->begin(); it != mgr.m_StrategyMap->end(); ++it)
	{
		m_StrategyMap->insert(std::make_pair(it->first, it->second));
	}
}
ServiceMgr::~ServiceMgr()
{
	if (m_DLLMap)
	{
		for (std::map<std::string, CDllHelper*>::iterator it = m_DLLMap->begin(); it != m_DLLMap->end(); ++it)
		{
			if (it->second)
			{
				delete it->second;
			}
		}	

		delete m_DLLMap;
		m_DLLMap = nullptr;
	}

	if (m_GateWayMap)
	{
		for (std::map<std::string, IGateway*>::iterator it = m_GateWayMap->begin(); it != m_GateWayMap->end(); ++it)
		{
			if (it->second)
			{
				delete it->second;
			}
		}

		delete m_GateWayMap;
		m_GateWayMap = nullptr;
	}

	if (m_StrategyMap)
	{
		for (std::map<std::string, IStrategy*>::iterator it = m_StrategyMap->begin(); it != m_StrategyMap->end(); ++it)
		{
			if (it->second)
			{
				delete it->second;
			}
		}

		delete m_StrategyMap;
		m_StrategyMap = nullptr;
	}
}

//从json文件解析gateway
bool ServiceMgr::initGateway()	
{
	std::stringstream settingStream;

	bool isRight = readFile(CKTRADER_SETTING_FILE, settingStream);

	if (!isRight)
	{
		return false;
	}

	std::string err;
	auto setting_json = json11::Json::parse(settingStream.str(), err);
	if (!err.empty())
	{
		return false;
	}

	auto gatewaySeting = setting_json["Gateway"];

	for (auto& k : gatewaySeting.array_items())
	{
		std::string name = k["name"].string_value();
		std::string address = k["fileAddress"].string_value();

		loadGateWay(name, address);
	}

	return true;
}
//从dll动态加载gateway
IGateway* ServiceMgr::loadGateWay(std::string name, std::string path)
{
	std::map<std::string, IGateway*>::iterator it;
	it = m_GateWayMap->find(name);
	if (it != m_GateWayMap->end())
	{
		return it->second;
	}

	CDllHelper *_dll = new CDllHelper(path.c_str());
	m_DLLMap->insert(std::make_pair(name, _dll));

	CreateGateway pfunc = _dll->GetProcedure<CreateGateway>("CreateGateway");
	if (pfunc)
	{
		IGateway* gate = pfunc(name.c_str());		
		m_GateWayMap->insert(std::make_pair(name, gate));

		return gate;
	}

	return nullptr;
}

//从json文件解析strategy
bool ServiceMgr::initStrategy()
{
	std::stringstream settingStream;

	bool isRight = readFile(CKTRADER_SETTING_FILE, settingStream);

	if (!isRight)
	{
		return false;
	}

	std::string err;
	auto setting_json = json11::Json::parse(settingStream.str(), err);

	auto gatewaySeting = setting_json["strategy"];

	for (auto& k : gatewaySeting.array_items())
	{
		std::string name = k["name"].string_value();
		std::string address = k["fileAddress"].string_value();

		loadGateWay(name, address);
	}

	return true;
}

//从dll动态加载strategy
IStrategy* ServiceMgr::loadStrategy(std::string name,std::string path)
{
	std::map<std::string, IStrategy*>::iterator it;
	it = m_StrategyMap->find(name);
	if (it != m_StrategyMap->end())
	{
		return it->second;
	}

	CDllHelper *_dll = new CDllHelper(_FTA(path.c_str()));
	m_DLLMap->insert(std::make_pair(name, _dll));

	CreateStrategy pfunc = _dll->GetProcedure<CreateStrategy>("CreateStrategy");
	if (pfunc)
	{
		IStrategy* pStrategy = pfunc(this,name.c_str());
		m_StrategyMap->insert(std::make_pair(name, pStrategy));

		return pStrategy;
	}

	return nullptr;
}

//从本地字典内获取gateway
IGateway* ServiceMgr::getGateWay(std::string gateWayName)
{
	std::map<std::string, IGateway*>::iterator it;
	it = m_GateWayMap->find(gateWayName);
	if (it == m_GateWayMap->end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}
}

IStrategy* ServiceMgr::getStrategy(std::string strategyName)
{
	std::map<std::string, IStrategy*>::iterator it;
	it = m_StrategyMap->find(strategyName);
	if (it == m_StrategyMap->end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}
}

bool ServiceMgr::initStrategy(std::string strategyName)
{
	std::map<std::string, IStrategy*>::iterator it;
	it = m_StrategyMap->find(strategyName);
	if (it == m_StrategyMap->end())
	{
		return false;
	}
	else
	{
		return it->second->onInit();
	}
}

bool ServiceMgr::startStrategy(std::string strategyName)
{
	std::map<std::string, IStrategy*>::iterator it;
	it = m_StrategyMap->find(strategyName);
	if (it == m_StrategyMap->end())
	{
		return false;
	}
	else
	{
		return it->second->onStart();
	}
}

bool ServiceMgr::stopStrategy(std::string strategyName)
{
	std::map<std::string, IStrategy*>::iterator it;
	it = m_StrategyMap->find(strategyName);
	if (it == m_StrategyMap->end())
	{
		return false;
	}
	else
	{
		return it->second->onStop();
	}
}

bool ServiceMgr::readFile(std::string fileName,std::stringstream& stringText)
{
	std::ifstream is(fileName, std::ifstream::binary);

	bool ret_value = false;
	if (is) 
	{
		// get length of file:
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);

		char * buffer = new char[length];

		// read data as a block:
		is.read(buffer, length);

		if (is)
		{
			ret_value = true;
			stringText << buffer;
		}

		is.close();

		// ...buffer contains the entire file...
		delete[] buffer;
	}
	return ret_value;
}

bool ServiceMgr::writeFile(std::string fileName, std::stringstream& stringText)
{
	std::ofstream ofs(fileName, std::ofstream::out);

	ofs << stringText.str();

	ofs.close();

	return true;
}

std::map<std::string, IGateway*>* ServiceMgr::getGatewayMap()
{
	return m_GateWayMap;
}

std::map<std::string, IStrategy*>* ServiceMgr::getStrategyMap()
{
	return m_StrategyMap;
}

}//cktrader