#include "animation_engine.hpp"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::init()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::fini()
	{
		for (uint i = 0; i < m_animCtrls.size(); ++i)
			delete m_animCtrls[i];

		m_animCtrls.clear();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::animate(float dt)
	{

	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::postAnimate()
	{

	}

	//----------------------------------------------------------------------------------------------
	Handle AnimationEngine::addAnimDef(const AnimationData* animCtrl)
	{

	}

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::delAnimDef(Handle handle)
	{

	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::playAnim(Handle id, const char* name, bool isLooped, uint rate)
	{
		assert(id != BR_INVALID_HANDLE && id < m_animCtrls.size());
		
		AnimationData& data = *m_animCtrls[id];

		AnimationData::AnimLayer layer;
		layer.m_anim  = getAnim(name);
		layer.m_blend = 1.0f;
		layer.m_time  = 0.0f;

		data.m_animLayers.push_back(layer);
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::stopAnim(Handle id)
	{
		assert(id != BR_INVALID_HANDLE && id < m_animCtrls.size());

	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::blendAnim(
		Handle id, float srcBlend, float dstBlend, const char* name, bool isLooped, uint rate)
	{
		assert(id != BR_INVALID_HANDLE && id < m_animCtrls.size());

	}

	//----------------------------------------------------------------------------------------------
	Ptr<Animation> AnimationEngine::getAnim(const char* name)
	{
		AnimationsMap::iterator iter = m_animations.find(name);
		if (iter != m_animations.end())
		{
			return iter->second;
		}
		else
		{
			RODataPtr data = FileSystem::instance().readFile("resources/" + std::string(name));
			if (!data.get())
			{
				return NULL;
			}

			Ptr<Animation> result = new Animation();
			if (result->load(*data))
			{
				m_animations[name] = result;
			}
			else
			{
				result = NULL;
			}
			return result;
		}
	}

} //-- render
} //-- brUGE