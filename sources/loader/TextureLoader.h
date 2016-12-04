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

		std::shared_ptr<render::ITexture> loadTex2D(const utils::ROData& data);

	private:
		uint m_totalSize;
	};

} // brUGE
