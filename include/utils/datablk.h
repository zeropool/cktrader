#pragma once
#ifndef __DATABLK_H__
#define __DATABLK_H__

#include <utility>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <typeinfo>

#include "ckdef.h"

struct CK_EXPORTS Datablk
{
	Datablk(void) : m_tpIndex(std::type_index(typeid(void))) {}
	Datablk(const Datablk& that) : m_ptr(that.Clone()), m_tpIndex(that.m_tpIndex) {}
	Datablk(Datablk && that) : m_ptr(std::move(that.m_ptr)), m_tpIndex(that.m_tpIndex) {}

	//创建智能指针时，对于一般的类型，通过std::decay来移除引用和cv符，从而获取原始类型
	template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Datablk>::value, U>::type> 
	Datablk(U && value) :m_ptr(new Derived < typename std::decay<U>::type>(std::forward<U>(value))),
						m_tpIndex(std::type_index(typeid(typename std::decay<U>::type))) {}

	bool IsNull() const 
	{
		return !bool(m_ptr); 
	}

	template<class U> bool Is() const
	{
		return m_tpIndex == std::type_index(typeid(U));
	}

	//将datablk转换为实际的类型
	template<class U>
	U& cast()
	{
		if (!Is<U>())
		{
			throw std::bad_cast();
		}

		auto derived = dynamic_cast<Derived<U>*> (m_ptr.get());
		return derived->m_value;
	}

	Datablk& operator=(const Datablk& a)
	{
		if (m_ptr == a.m_ptr)
		{
			return *this;
		}

		m_ptr = a.Clone();
		m_tpIndex = a.m_tpIndex;
		return *this;
	}

private:
	struct Base;
	typedef std::unique_ptr<Base> BasePtr;

	struct Base
	{
		virtual ~Base() {}
		virtual BasePtr Clone() const = 0;
	};

	template<typename T>
	struct Derived : Base
	{
		template<typename U>
		Derived(U && value) : m_value(std::forward<U>(value)) { }

		BasePtr Clone() const
		{
			return BasePtr(new Derived<T>(m_value));
		}

		T m_value;
	};

	BasePtr Clone() const
	{
		if (m_ptr != nullptr)
			return m_ptr->Clone();

		return nullptr;
	}

	BasePtr m_ptr;
	std::type_index m_tpIndex;
};

#endif
