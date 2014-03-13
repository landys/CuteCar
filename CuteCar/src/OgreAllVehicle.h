#pragma once

#include "Define.h"
#include <vector>
#include <Ogre.h>
#include <string>
//*************** temp
struct AllVehicleInfoOgre
{
	Ogre::Vector3 vepos;
	Ogre::Quaternion verot;
	Ogre::Vector3 campos;
	Ogre::Quaternion camrot;
	float angle0;
	float angle1;
	float angle2;
	float angle3;
	std::string name;
};

class OgreWheel
{
protected:
	OgreWheel(void);
public:
	static OgreWheel* createOgreWheel(Ogre::SceneNode* parNode, Ogre::Vector3 pos);	// ��������
	void updateWheel(const float angle);	// ��������
	virtual ~OgreWheel(void);
private:
	Ogre::SceneNode* mParNode;	// �����
	Ogre::SceneNode* mNode;	// ������
	static int mWheelCount;	// ����Ϊ�˽������ʱ����ͻ
};

class OgreVehicle
{
protected:
	OgreVehicle(void);
public:
	static OgreVehicle* createOgreVehicle(Ogre::SceneNode* parNode, std::string name, Ogre::Vector3 pos);	// ��������
	void updateVehicle(const AllVehicleInfoOgre& allv);	// ���³���
	std::string getVehicleName();	// �õ��ó��û�����
	bool hasCamera();	// �Ƿ�ӵ�������
	void attachCamera(Ogre::Camera* cam);	// ʹ�ó�ӵ�������
	void setCamera(const Ogre::Vector3& repos, const Ogre::Quaternion& reorien);	// ���øó�������ӽ�
	void removeCamera();	// �Ƴ��ó��������
	bool addVehicleItem(ItemType type);	// ��ӵ���
	ItemType getCurrentItem();	// �õ���ǰ����
	ItemType removeCurrentItem();	// ���ص�ǰ���ߺ��Ƴ�,���Ƶ���һ������λ�ô�
	std::vector<ItemType>* getAllItem();
	void setToBeginning();
	virtual ~OgreVehicle(void);
private:
	Ogre::SceneNode* mParNode;	// �����
	Ogre::SceneNode* mNode;	// ������
	Ogre::Camera* mCam;	// �����
	OgreWheel* mWheels[4];	// �ĸ�����
	std::string mName;	// �û�����
	std::vector<ItemType> mItems;
	int miCurItem;
};

class OgreAllVehicle
{
public:
	static OgreAllVehicle* getSingleton();	// �õ�Ψһ�ĳ�������Ķ���
	OgreVehicle* addVehicle(Ogre::SceneNode* ogreRoot, std::string name, Ogre::Vector3 pos);	// ���һ����
	void updateAllVehicle(AllVehicleInfoOgre* allvehicle);	// �������г���Ϣ**************** temp
	OgreVehicle* getUserVehicle(const std::string& name);	// �õ��û���Ϊname���û��ĳ�
	OgreVehicle* RemoveVehicleCam();	// ʹ��ǰ�ӽǵĳ��ͷ������,���ҷ�������һ����
	//void setToBeginning(const std::string& name);
	virtual ~OgreAllVehicle(void);
protected:
	OgreAllVehicle(void);
private:
	static OgreAllVehicle* mSingleton;
	std::vector<OgreVehicle*> mAllVehicle;	// ������
};

class OgreTerrainItem
{
public:
	static OgreTerrainItem* createOgreTerrainItem(Ogre::SceneNode* parNode, const Ogre::Vector3& pos, ItemType type, int id);
	bool turnTerrainItem(int id, float angle=1.0f);	// תy�ἴ��ֱ������ת����,�����id���ɹ�����true 
	bool removeTerrainItem(int id);	// �Ƴ�����,�����id���ɹ�����true
	int getTerrainItemId();	// �õ��õ��ߵ�ID
	virtual ~OgreTerrainItem(void);
private:
	Ogre::SceneNode* mParNode;	// �����
	Ogre::SceneNode* mNode;	// ������
	int mId;
	ItemType mItemType;
	Ogre::Vector3 mRandPlace[10];
protected:
	OgreTerrainItem(Ogre::SceneNode* parNode, ItemType type, int id);
};