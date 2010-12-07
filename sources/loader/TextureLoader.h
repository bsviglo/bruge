#pragma once

#include "render/ITexture.h"

namespace brUGE
{
	namespace utils
	{
		class ROData;
	}

	//----------------------------------------------------------------------------------------------
	class TextureLoader
	{
	public:
		TextureLoader();
		~TextureLoader();

		bool init();
		void shutdown();

		Ptr<render::ITexture> loadTex2D(const utils::ROData& data);

	private:
		uint m_totalSize;
	};

} // brUGE
