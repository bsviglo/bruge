#ifndef _BR_OGLVERTEXBUFFER_H_
#define _BR_OGLVERTEXBUFFER_H_

#include "br_OGLCommon.h"
#include "render/ibr_Buffer.h"

namespace brUGE
{
	namespace render
	{	
		// Честно говоря не знаю на что это прием похож с точки зрения структурных паттернов.
		// Мне кажется что это нечто типа полу Bridge полу Adapter. В любом случае он неплохо
		// работает, и позволяет давольно красиво качественно и безопасно вынести общий код для
		// всех классов(это лучший варинат из мной рассмотренных, по-крайней мере пока.)
		// TODO: подумать как можно избавиться от необходимости реализовывать вызов виртуальных
		// функций простым вызовом имплементирующей функции. Вполне возможно что это дело можно
		// автоматизировать.
		//------------------------------------------
		class brOGLBufferImpl
		{
		public:
			virtual ~brOGLBufferImpl();

			void* mapImpl(ibrBuffer::ebrAccessFlag access);
			void unMapImpl();

			bool isMappedImpl() const;
			uint getSizeImpl() const;
			void* getMapPointerImpl() const;

		protected:
			brOGLBufferImpl(ibrBuffer::ebrUsageFlag usage, GLenum bufferType);

			static GLenum _getOGLUsage(ibrBuffer::ebrUsageFlag usage);
			static GLenum _getOGLAccess(ibrBuffer::ebrAccessFlag access);

		protected:
			GLuint bufferId_;
			GLenum bufferType_;
			GLenum bufferUsage_;
		};

		// 
		//------------------------------------------
		class brOGLVertexBuffer :	public ibrVertexBuffer,
									private brOGLBufferImpl
		{

			friend class brOGLRenderDevice;

		public:
			brOGLVertexBuffer(ibrBuffer::ebrUsageFlag usage);
			brOGLVertexBuffer(uint vertexCount, uint vertexSize,
								ibrBuffer::ebrUsageFlag usage, const void* data);
			virtual ~brOGLVertexBuffer();

			virtual void* map(ebrAccessFlag access);
			virtual void unMap();

			virtual bool isMapped() const;
			virtual uint getSize() const;
			virtual void* getMapPointer() const;
		};

		// 
		//------------------------------------------
		class brOGLIndexBuffer :	public ibrIndexBuffer,
									private brOGLBufferImpl
		{

			friend class brOGLRenderDevice;						

		public:
			brOGLIndexBuffer(ibrBuffer::ebrUsageFlag usage);
			brOGLIndexBuffer(uint indexCount, ebrIndexType indexType,
				ibrBuffer::ebrUsageFlag usage, const void* data);
			virtual ~brOGLIndexBuffer();

			virtual void* map(ibrBuffer::ebrAccessFlag access);
			virtual void unMap();

			virtual bool isMapped() const;
			virtual uint getSize() const;
			virtual void* getMapPointer() const;
		};
		
		// TODO: Реализация.
		class brOGLConstBuffer :	public ibrConstantBuffer
		{

		};

	}
}

#endif /*_BR_OGLVERTEXBUFFER_H_*/