#ifndef _BR_OGLINDEXBUFFER_H_
#define _BR_OGLINDEXBUFFER_H_

#include "br_OGLCommon.h"
#include "render/ibr_Buffer.h"

namespace brUGE
{
	namespace render
	{
		// 
		//------------------------------------------
		class brOGLIndexBuffer : public ibrIndexBuffer
		{
		public:
			brOGLIndexBuffer(uint indexCount, ebrIndexType indexType,
				ibrBuffer::ebrUsageFlag usage, const void* data);
			virtual ~brOGLIndexBuffer();

			virtual void* map(ibrBuffer::ebrAccessFlag access);
			virtual void unMap();

			virtual bool isMapped() const;
			virtual uint getSize() const;

			void* getMapPointer() const;
			GLuint getOGLBufferId() const { return bufferId_; }

		private:
			GLuint bufferId_;
		};

	}
}

#endif /*_BR_OGLINDEXBUFFER_H_*/