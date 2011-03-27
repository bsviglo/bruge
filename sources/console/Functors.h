#pragma once

#include "prerequisites.hpp"
#include "utils/ArgParser.h"
#include <string>
#include <vector>
#include <cassert>

namespace brUGE
{
	using utils::parseTo;

	typedef std::vector<std::string> ParamList;

	// 
	//------------------------------------------
	class Functor
	{
	public:
		virtual ~Functor() {}
		virtual void operator() (const ParamList& param) = 0;
	};

	// 
	//------------------------------------------
	class FunctorFunc : public Functor
	{
	public:
		FunctorFunc(int (*f)())
			//-- ToDo: reconsider.
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn, reinterpret_cast<TypelessFunction>(f))) {}

		template <typename T1>
		FunctorFunc(int (*f)(T1))
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn<T1>, reinterpret_cast<TypelessFunction>(f))) {}

		template <typename T1, typename T2>
		FunctorFunc(int (*f)(T1, T2))
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn<T1,T2>, reinterpret_cast<TypelessFunction>(f))) {}

		template <typename T1, typename T2, typename T3>
		FunctorFunc(int (*f)(T1, T2, T3))
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn<T1,T2,T3>, reinterpret_cast<TypelessFunction>(f))) {}

		template <typename T1, typename T2, typename T3, typename T4>
		FunctorFunc(int (*f)(T1, T2, T3, T4))
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn<T1,T2,T3,T4>, reinterpret_cast<TypelessFunction>(f))) {}

		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		FunctorFunc(int (*f)(T1, T2, T3, T4, T5))
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn<T1,T2,T3,T4,T5>, reinterpret_cast<TypelessFunction>(f))) {}
		
		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		FunctorFunc(int (*f)(T1, T2, T3, T4, T5, T6))
			:	m_command(std::make_pair<ProcFunction, TypelessFunction>(proc_fn<T1,T2,T3,T4,T5,T6>, reinterpret_cast<TypelessFunction>(f))) {}


		virtual void operator() (const ParamList& param)
		{
			m_command.first(m_command.second, param);
		}

	private:
		typedef void (*TypelessFunction)();
		typedef int	(*ProcFunction)(TypelessFunction f, const ParamList& param);
		typedef std::pair<ProcFunction, TypelessFunction> ProcFunctionPair;

		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 0)	throw std::runtime_error("no args are expected.");
			return reinterpret_cast<int(*)()>(f)();
		}

		template <typename T1>
		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 1)	throw std::runtime_error("1 arg is expected.");
			return reinterpret_cast<int(*)(T1)>(f)(parseTo<T1>(param[0]));
		}

		template <typename T1, typename T2>
		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 2)	throw std::runtime_error("2 args are expected.");
			return reinterpret_cast<int(*)(T1,T2)>(f)(parseTo<T1>(param[0]), parseTo<T2>(param[1]));
		}

		template <typename T1, typename T2, typename T3>
		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 3)	throw std::runtime_error("3 args are expected.");
			return reinterpret_cast<int(*)(T1,T2,T3)>(f)(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]));
		}

		template <typename T1, typename T2, typename T3, typename T4>
		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 4)	throw std::runtime_error("4 args are expected.");
			return reinterpret_cast<int(*)(T1,T2,T3,T4)>(f)(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]), parseTo<T4>(param[3]));
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 5)	throw std::runtime_error("5 args are expected.");
			return reinterpret_cast<int(*)(T1,T2,T3,T4,T5)>(f)(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]), parseTo<T4>(param[3]), parseTo<T5>(param[4]));
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		static int proc_fn(TypelessFunction f, const ParamList& param)
		{
			if(param.size() != 6)	throw std::runtime_error("6 args are expected.");
			return reinterpret_cast<int(*)(T1,T2,T3,T4,T5,T6)>(f)(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]), parseTo<T4>(param[3]), parseTo<T5>(param[4]), parseTo<T6>(param[5]));
		}

		ProcFunctionPair m_command;
	};

	// 
	//------------------------------------------
	template<typename OBJ>
	class FunctorMethod : public Functor
	{
	public:
		FunctorMethod(OBJ* obj, int (OBJ:: *m)())
			:	m_object(obj),
				//-- ToDo: reconsider.
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md, reinterpret_cast<TypelessMethod>(m))) {}

		template <typename T1>
		FunctorMethod(OBJ* obj, int (OBJ:: *m)(T1))
			:	m_object(obj),
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md<T1>, reinterpret_cast<TypelessMethod>(m))) {}

		template <typename T1, typename T2>
		FunctorMethod(OBJ* obj, int (OBJ:: *m)(T1, T2))
			:	m_object(obj),
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md<T1,T2>, reinterpret_cast<TypelessMethod>(m))) {}

		template <typename T1, typename T2, typename T3>
		FunctorMethod(OBJ* obj, int (OBJ:: *m)(T1, T2, T3))
			:	m_object(obj),
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md<T1,T2,T3>, reinterpret_cast<TypelessMethod>(m))) {}

		template <typename T1, typename T2, typename T3, typename T4>
		FunctorMethod(OBJ* obj, int (OBJ:: *m)(T1, T2, T3, T4))
			:	m_object(obj),
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md<T1,T2,T3,T4>, reinterpret_cast<TypelessMethod>(m))) {}

		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		FunctorMethod(OBJ* obj, int (OBJ:: *m)(T1, T2, T3, T4, T5))
			:	m_object(obj),
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md<T1,T2,T3,T4,T5>, reinterpret_cast<TypelessMethod>(m))) {}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		FunctorMethod(OBJ* obj, int (OBJ:: *m)(T1, T2, T3, T4, T5, T6))
			:	m_object(obj),
				m_command(std::make_pair<ProcMethod, TypelessMethod>(proc_md<T1,T2,T3,T4,T5,T6>, reinterpret_cast<TypelessMethod>(m))) {}

		virtual void operator() (const ParamList& param)
		{
			assert(m_object && "Method was called after destroying its object.");
			m_command.first(m_object, m_command.second, param);
		}

	private:
		typedef void (OBJ:: *TypelessMethod)();
		typedef int (*ProcMethod)(OBJ* obj, TypelessMethod m, const ParamList& param);
		typedef std::pair<ProcMethod, TypelessMethod> ProcMethodPair;

		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 0)	throw std::runtime_error("no args are expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)()>(m))();
		}

		template <typename T1>
		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 1)	throw std::runtime_error("1 arg is expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)(T1)>(m))(parseTo<T1>(param[0]));
		}

		template <typename T1, typename T2>
		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 2)	throw std::runtime_error("2 arg is expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)(T1,T2)>(m))(parseTo<T1>(param[0]), parseTo<T2>(param[1]));
		}

		template <typename T1, typename T2, typename T3>
		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 3)	throw std::runtime_error("3 arg is expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)(T1,T2,T3)>(m))(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]));
		}

		template <typename T1, typename T2, typename T3, typename T4>
		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 4)	throw std::runtime_error("4 arg is expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)(T1,T2,T3,T4)>(m))(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]), parseTo<T4>(param[3]));
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 5)	throw std::runtime_error("5 arg is expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)(T1,T2,T3,T4,T5)>(m))(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]), parseTo<T4>(param[3]), parseTo<T5>(param[4]));
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		static int proc_md(OBJ* obj, TypelessMethod m, const ParamList& param)
		{
			if(param.size() != 6)	throw std::runtime_error("6 arg is expected.");
			return (obj->*reinterpret_cast<int(OBJ:: *)(T1,T2,T3,T4,T5,T6)>(m))(parseTo<T1>(param[0]), parseTo<T2>(param[1]), parseTo<T3>(param[2]), parseTo<T4>(param[3]), parseTo<T5>(param[4]), parseTo<T6>(param[5]));
		}

		ProcMethodPair	m_command;
		OBJ*			m_object;
	};

} // brUGE
