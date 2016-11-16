#pragma once
#ifndef __CONCURRENTQUEUE_H__
#define __CONCURRENTQUEUE_H__

#include <mutex>
#include <queue>
#include <condition_variable>

namespace cktrader{
///线程安全的队列
template<class Data>
class ConcurrentQueue
{
private:
	std::queue<Data> the_queue;								//标准库队列
	mutable std::recursive_mutex the_mutex;							//互斥锁
	mutable std::condition_variable_any the_condition_variable;			//条件变量

public:
	//存入新的任务
	void push(Data const& data)
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);				//获取互斥锁
		the_queue.push(data);							//向队列中存入数据

		the_condition_variable.notify_one();			//通知正在阻塞等待的线程
	}

	//检查队列是否为空
	bool empty() const
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		bool isEmpty = the_queue.empty();

		return isEmpty;
		
	}

	//取出
	Data wait_and_pop()
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);

		while (the_queue.empty())						//当队列为空时
		{
			the_condition_variable.wait(lck);			//等待条件变量通知
		}

		Data popped_value;
		popped_value = the_queue.front();			//获取队列中的最后一个任务
		the_queue.pop();								//删除该任务

		return popped_value;							//返回该任务
	}

	size_t size()
	{
		std::unique_lock<std::recursive_mutex> lck(the_mutex);
		return the_queue.size();
	}
};

}//cktrader

#endif