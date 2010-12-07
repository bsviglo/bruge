#pragma once

#include "render_common.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class IBuffer : public utils::RefCount
	{
	public:
		
		//-- buffer pipeline type.
		enum EType
		{
			TYPE_VERTEX = 0,
			TYPE_INDEX,
			TYPE_UNIFORM,
			TYPE_TEXTURE
		};

		//-- buffer usage type.
		enum EUsage
		{
			USAGE_DEFAULT = 0, //-- один раз залили в GPU где-то раз 10 отрендерели.
			USAGE_IMMUTABLE,   //-- раз залили в GPU и больше не трогаем.
			USAGE_DYNAMIC,	   //-- один раз залили в GPU один раз отрендерели.
			USAGE_STAGING	   //-- ToDo: пока не поддерживается. Надо над этим еще подумать.
		};

		//-- mapping flags:
		//-- Если установлен флаг CPU_AF_WRITE это означает что при вызове функции map
		//-- валидны только AF_WRITE, AF_WRITE_DISCARD и AF_WRITE_NO_OVERWRITE.
		//-- Если CPU_AF_NONE мапить вообще нельзя.	
		//-- Если CPU_AF_READ только AF_READ.
		//-- Флаги можно комбинировать.
		enum ECPUAccess
		{
			CPU_ACCESS_NONE	= 0,
			CPU_ACCESS_READ,
			CPU_ACCESS_WRITE,
			CPU_ACCESS_READ_WRITE
		};

		//-- Флаги доступа к буфферу для центрального процессора и видеокарты.
		enum EAccess
		{
			ACCESS_READ	= 0,
			ACCESS_WRITE,
			ACCESS_READ_WRITE,
			ACCESS_WRITE_DISCARD,
			ACCESS_WRITE_NO_OVERWRITE
		};

	public:
		template<typename T>
		T* map(EAccess access) { return static_cast<T*>(doMap(access)); }
		void  unmap() { doUnmap(); }
		
		uint  getElemCount() const { return m_elemCount; }
		uint  getElemSize() const { return m_elemSize; }
		uint  getSize() const { return m_elemCount * m_elemSize; }
		EType getType() const { return m_type; }

	protected:
		virtual ~IBuffer() { }
		IBuffer(EType type, EUsage usage, ECPUAccess cpuAccess)
			: m_elemCount(0), m_elemSize(0), m_type(type), m_usage(usage), m_cpuAccess(cpuAccess) {}

		virtual void* doMap(EAccess access) = 0;
		virtual void  doUnmap() = 0;

		uint	   m_elemSize; //-- size of one element. e.q. sizeof(elem)
		uint	   m_elemCount; //-- count of elements in the buffer.
		EType	   m_type;
		EUsage	   m_usage;
		ECPUAccess m_cpuAccess;
	};

} // render
} // brUGE
