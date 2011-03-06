#pragma once

namespace brUGE
{
namespace utils
{

	//-- Make derived class non-copyable.
	//----------------------------------------------------------------------------------------------
	class NonCopyable
	{
	public:
		NonCopyable() { }
		~NonCopyable() { }

	private:
		NonCopyable(const NonCopyable&);
		NonCopyable& operator = (const NonCopyable&);
	};

} //-- utils
} //-- brUGE