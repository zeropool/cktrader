#include "eventservice/eventengine.h"

#include <memory>
#include <chrono>
#include <algorithm>

namespace cktrader {

EventEngine::EventEngine():the_mutex(), m_active(false)
{
	m_task_thread_pool = nullptr;

	task_queue = new ConcurrentQueue<Task>;//事件任务队列
	
	handlers = new std::multimap<std::string, Handler>;

	m_timer_thread = nullptr;;//for timer
}

EventEngine::EventEngine(EventEngine& engine):the_mutex(), m_active(false)
{
	m_task_thread_pool = engine.m_task_thread_pool;

	task_queue = engine.task_queue;//事件任务队列

	handlers = engine.handlers;

	m_timer_thread = engine.m_timer_thread;//for timer
}

EventEngine::~EventEngine()
{
	std::lock_guard<std::recursive_mutex> lck(the_mutex);

	m_active = false;

	//删除任务处理线程池
	if (m_task_thread_pool)
	{		
		for (std::vector<std::thread*>::iterator it = m_task_thread_pool->begin(); it != m_task_thread_pool->end(); it++)
		{
			(*it)->join();
			delete (*it);
			m_task_thread_pool->erase(it);
		}

		delete m_task_thread_pool;
		m_task_thread_pool = nullptr;
	}

	if (m_timer_thread)
	{
		m_timer_thread->join();
		delete m_timer_thread;
		m_timer_thread = nullptr;
	}	

	if (task_queue)
	{
		delete task_queue;
		task_queue = nullptr;
	}
	
	if (handlers)
	{
		delete handlers;
		handlers = nullptr;
	}	
}

bool EventEngine::registerHandler(std::string type, Handler f)
{
	if (!handlers)
	{
		return false;
	}

	handlers->insert(std::make_pair(type, f));
	return true;
}

bool EventEngine::unRegisterHandler(std::string type)
{
	if (!handlers)
	{
		return false;
	}
	
	handlers->erase(type);
	return true;
}

bool EventEngine::put(Task &t)
{
	if (!task_queue)
	{
		return false;
	}

	task_queue->push(t);
	return true;
}

bool EventEngine::startEngine()
{
	if (m_active || m_task_thread_pool || m_timer_thread)
	{
		return false;
	}

	m_active = true;

	m_task_thread_pool = new std::vector<std::thread*>;

	std::function<void()> f = std::bind(&EventEngine::processTask, this);

	for (unsigned i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		std::thread* thread_worker = new std::thread(f);
		m_task_thread_pool->push_back(thread_worker);
	}

	std::function<void()> f_timer = std::bind(&EventEngine::trigerTimer, this);
	m_timer_thread = new std::thread(f_timer);

	return true;
}

bool EventEngine::stopEngine()
{
	if (!m_active || !m_task_thread_pool || !m_timer_thread)
	{
		return false;
	}

	m_active = false;

	if (m_timer_thread)
	{
		m_timer_thread->join();
		delete m_timer_thread;
		m_timer_thread = nullptr;
	}

	if (m_task_thread_pool)
	{
		for (std::vector<std::thread*>::iterator it = m_task_thread_pool->begin(); it != m_task_thread_pool->end(); it++)
		{
			(*it)->join();
			delete (*it);
			m_task_thread_pool->erase(it);
		}

		delete m_task_thread_pool;
		m_task_thread_pool = nullptr;
	}

	return true;
}

void EventEngine::processTask()
{
	while (m_active)
	{
		Task task;

		task = task_queue->wait_and_pop();

		if (task.type == EVENT_HANDLER)
		{
			FuncData func;

			try
			{
				func = task.task_data.cast<FuncData>();
				func.h(func.arg);
			}
			catch (std::bad_cast& bc)
			{

			}
		}else
		{
			std::pair <std::multimap<std::string, Handler>::iterator, std::multimap<std::string, Handler>::iterator> ret;
			ret = handlers->equal_range(task.type);

			for (std::multimap<std::string, Handler>::iterator it = ret.first; it != ret.second; ++it)
			{
				FuncData func;
				func.h = it->second;
				func.arg = task.task_data;

				Task handlerTask;
				handlerTask.type = EVENT_HANDLER;
				handlerTask.task_data = func;
				put(handlerTask);
			}//for			
		}//else
		
	}//while
}

void EventEngine::trigerTimer()
{
	while (m_active)
	{
		Task task = Task();
		task.type = EVENT_TIMER;
		task.task_data = Datablk();
		put(task);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

}//cktrader