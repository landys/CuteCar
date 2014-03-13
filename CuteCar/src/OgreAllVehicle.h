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
	static OgreWheel* createOgreWheel(Ogre::SceneNode* parNode, Ogre::Vector3 pos);	// 创建轮子
	void updateWheel(const float angle);	// 更新轮子
	virtual ~OgreWheel(void);
private:
	Ogre::SceneNode* mParNode;	// 父结点
	Ogre::SceneNode* mNode;	// 自身结点
	static int mWheelCount;	// 纯粹为了结点命名时不冲突
};

class OgreVehicle
{
protected:
	OgreVehicle(void);
public:
	static OgreVehicle* createOgreVehicle(Ogre::SceneNode* parNode, std::string name, Ogre::Vector3 pos);	// 创建车子
	void updateVehicle(const AllVehicleInfoOgre& allv);	// 更新车子
	std::string getVehicleName();	// 得到该车用户名字
	bool hasCamera();	// 是否拥有照相机
	void attachCamera(Ogre::Camera* cam);	// 使该车拥有照相机
	void setCamera(const Ogre::Vector3& repos, const Ogre::Quaternion& reorien);	// 设置该车照相机视角
	void removeCamera();	// 移除该车的照相机
	bool addVehicleItem(ItemType type);	// 添加道具
	ItemType getCurrentItem();	// 得到当前道具
	ItemType removeCurrentItem();	// 返回当前道具后移除,并移到下一个道具位置处
	std::vector<ItemType>* getAllItem();
	void setToBeginning();
	virtual ~OgreVehicle(void);
private:
	Ogre::SceneNode* mParNode;	// 父结点
	Ogre::SceneNode* mNode;	// 自身结点
	Ogre::Camera* mCam;	// 照相机
	OgreWheel* mWheels[4];	// 四个轮子
	std::string mName;	// 用户名字
	std::vector<ItemType> mItems;
	int miCurItem;
};

class OgreAllVehicle
{
public:
	static OgreAllVehicle* getSingleton();	// 得到唯一的车辆管理的对象
	OgreVehicle* addVehicle(Ogre::SceneNode* ogreRoot, std::string name, Ogre::Vector3 pos);	// 添加一辆车
	void updateAllVehicle(AllVehicleInfoOgre* allvehicle);	// 更新所有车信息**************** temp
	OgreVehicle* getUserVehicle(const std::string& name);	// 得到用户名为name的用户的车
	OgreVehicle* RemoveVehicleCam();	// 使当前视角的车释放照相机,并且返回其下一辆车
	//void setToBeginning(const std::string& name);
	virtual ~OgreAllVehicle(void);
protected:
	OgreAllVehicle(void);
private:
	static OgreAllVehicle* mSingleton;
	std::vector<OgreVehicle*> mAllVehicle;	// 车队列
};

class OgreTerrainItem
{
public:
	static OgreTerrainItem* createOgreTerrainItem(Ogre::SceneNode* parNode, const Ogre::Vector3& pos, ItemType type, int id);
	bool turnTerrainItem(int id, float angle=1.0f);	// 转y轴即竖直方向旋转道具,是这个id即可功返回true 
	bool removeTerrainItem(int id);	// 移除道具,是这个id即成功返回true
	int getTerrainItemId();	// 得到该道具的ID
	virtual ~OgreTerrainItem(void);
private:
	Ogre::SceneNode* mParNode;	// 父结点
	Ogre::SceneNode* mNode;	// 自身结点
	int mId;
	ItemType mItemType;
	Ogre::Vector3 mRandPlace[10];
protected:
	OgreTerrainItem(Ogre::SceneNode* parNode, ItemType type, int id);
};