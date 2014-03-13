#pragma once

#include "Define.h"
#include "OgreAllVehicle.h"
#include <Ogre.h>

class OgreTerrain
{
public:
	OgreTerrain(Ogre::SceneManager* sceneMgr);
	virtual ~OgreTerrain(void);
	void createOgreTerrain(void);	// Ogre地图场景
	void addUser(std::string name, Ogre::Vector3 pos);
	void updateAllOgre(AllVehicleInfoOgre* allvehicle);	//**************** temp
	int userCamera(Ogre::Camera* cam, const std::string& name);
	void wholeCamera(Ogre::Camera* cam);	// 全局视角
	int switchCamera(Ogre::Camera* cam);	// 转换视角到下一个用户
	void addVehicleItem(ItemType type, const std::string& name);	// 捡到道具后添加到车的列表中
	void addTerrainItem(const Ogre::Vector3& pos, ItemType type, int id);	// 添加道具到场景中
	void removeTerrainItem(int id);	// 删除场景中道具
	void turnAllTerrainItem(float angle=1.0f);	// 转y轴即竖直方向旋转所有道具
	ItemType getCurrentVehicleItem(const std::string& name);	// 得到当前道具
	std::vector<ItemType>* getAllItem(const std::string& name);
	ItemType removeCurrentVehicleItem(const std::string& name);	// 返回当前道具后移除,并移到下一个道具位置处
	void addScratchLine();	// 添加起跑线
	void setToBeginning(const std::string& name);	// 恢复值到刚创建时
private:
	std::vector<OgreTerrainItem*> mTerrainItems;
	Ogre::SceneManager* mOgreSceneMgr;
	Ogre::SceneNode* mOgreRoot;	// 除地图之外其他物体的根结点
};
