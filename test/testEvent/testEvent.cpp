// testEvent.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "eventservice/EventEngine.h"

#include <iostream>

using namespace cktrader;

void timer(Datablk& tick)
{
	std::cout << "timer" << std::endl;
}
int main()
{
	EventEngine envet;
	envet.startEngine();
	std::cout << "register" << std::endl;
	envet.registerHandler(EVENT_TIMER, std::bind(timer,std::placeholders::_1));
	getchar();
    return 0;
}

 

