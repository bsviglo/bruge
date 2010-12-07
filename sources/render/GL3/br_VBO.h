#ifndef _BR_VBO_H_
#define _BR_VBO_H_

#include "br_OGLCommon.h"

namespace brUGE
{
	namespace render
	{
		
		//------------------------------------------------------------
		class brVBO
		{
		public:

			enum brVBOType
			{
				VBOT_INDICES,
				VBOT_VERTICES,
				VBOT_USERDEFINED //WARNING: behavior yet not defined
				// используется пока для всего что не индексы, т.е. текстурные координаты
				// нормали и тому подобное.
			};

		public:
			brVBO(brVBOType type);
			~brVBO();
			
			void bind() const;
			void unbind() const;
			bool create();
			void allocate(uint32 size);
			void allocateAndWriteData(const void *buf, uint32 size);
			void writeData(const void *buf, uint32 size, uint32 offset);
			uint32 size() { return size_; }

		protected:
			void _destroy();

		private:
			uint32	  target_;
			uint32	  id_;
			uint32	  size_;
			brVBOType type_;
		};

	}
}

#endif/*_BR_VBO_H_*/