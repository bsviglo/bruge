#pragma once

namespace brUGE
{

	class ISceneItem : utils::RefCount
	{
	public:


	};
	
	//
	//----------------------------------------------------------------------------------------------
	class SceneManager : public utils::Singleton<SceneManager>
	{
	public:
		typedef std::vector<Ptr<ISceneItem> > ItemsArray;

	public:
		SceneManager();
		~SceneManager();
		
		//-- 
		void addItem(const Ptr<ISceneItem>& item);
		void delItem(const Ptr<ISceneItem>& item);
		
		//--
		void loadMap(const std::string& name);
		void saveMap();
		void clenup();
		
		//--
		void update(float dt);
		
		//--
		void extract(const RenderCamera& camera, ItemsArray& iArray);

	private:
		ItemsArray m_items;
	};

} // brUGE