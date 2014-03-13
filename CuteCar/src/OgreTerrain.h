#pragma once

#include "Define.h"
#include "OgreAllVehicle.h"
#include <Ogre.h>

class OgreTerrain
{
public:
	OgreTerrain(Ogre::SceneManager* sceneMgr);
	virtual ~OgreTerrain(void);
	void createOgreTerrain(void);	// Ogre��ͼ����
	void addUser(std::string name, Ogre::Vector3 pos);
	void updateAllOgre(AllVehicleInfoOgre* allvehicle);	//**************** temp
	int userCamera(Ogre::Camera* cam, const std::string& name);
	void wholeCamera(Ogre::Camera* cam);	// ȫ���ӽ�
	int switchCamera(Ogre::Camera* cam);	// ת���ӽǵ���һ���û�
	void addVehicleItem(ItemType type, const std::string& name);	// �񵽵��ߺ���ӵ������б���
	void addTerrainItem(const Ogre::Vector3& pos, ItemType type, int id);	// ��ӵ��ߵ�������
	void removeTerrainItem(int id);	// ɾ�������е���
	void turnAllTerrainItem(float angle=1.0f);	// תy�ἴ��ֱ������ת���е���
	ItemType getCurrentVehicleItem(const std::string& name);	// �õ���ǰ����
	std::vector<ItemType>* getAllItem(const std::string& name);
	ItemType removeCurrentVehicleItem(const std::string& name);	// ���ص�ǰ���ߺ��Ƴ�,���Ƶ���һ������λ�ô�
	void addScratchLine();	// ���������
	void setToBeginning(const std::string& name);	// �ָ�ֵ���մ���ʱ
private:
	std::vector<OgreTerrainItem*> mTerrainItems;
	Ogre::SceneManager* mOgreSceneMgr;
	Ogre::SceneNode* mOgreRoot;	// ����ͼ֮����������ĸ����
};
