#pragma once

#include "Define.h"

#include "NxAllVehicle.h"
#include "NxTerrainItem.h"
#include <NxPhysics.h>
#include <NxCooking.h>


enum MatEnum 
{
	MAT_DEFAULT = 0,
	MAT_WOOD = 1,
	MAT_METAL = 2,
	MAT_RUBBER = 3,
	MAT_PLASTIC = 4
};


class NxTerrain
{
public:
	NxTerrain();
	virtual ~NxTerrain(void);
public:
	void initNx();	// 初始化Novodex
	NxReal chooseTrigMaterial(NxU32 faceIndex, NxReal lastSteepness);	// 根据地面倾斜角选择物理材质
	void createNxTerrain(void);	// 创建物理地图
	void simulate(float time, int steering, int acceleration, bool handBrake);	// 运行物理引擎功能
	
	void control(int steering, int acceleration, bool handBrake);	// 控制车辆运动
	void standUp();	// 车辆运动初始化
	void setToBeginning(const NxVec3& pos);
	void addUser(const NxVec3& pos, VehicleInfo vinfo, std::string name);	// 添加一个用户,这里实现的是添加一辆车
	//void addTerrainItem(ItemType type, const NxVec3& pos, float size);
	AllVehicleInfoNx* getAllVehicleInfo();	// 得到所有车辆的用于OGRE绘制的信息
	float getVelocity();	// 得到车辆速度

	int getNumUser()	// 得到总用户数目
	{
		return mAllVehicle->getNumUser();
	}
	int userControl(const std::string& name);	// 控制用户
	int getOilAmount();
	void setOilAmount(int oilAmount);
	int getLoopNum();
	void setLoopNum(int nLoop);
	void useSlipProp();	// 使用打滑道具
	void cancelSlipProp();	// 去除打滑道具
	void useFlashProp();	// 使用闪光道具
	void cancelFlashProp();	// 去除闪光道具
	void setMaxVelocity(float velocity);
	NxScene* getNxScene()	// 得到物理场景
	{
		return m_NxScene;
	}

	// 道具封装
	void initTerrainItem(NxScene* scene)
	{
		NxTerrainItem::initTerrainItem(scene);
	}

	NxTerrainItem* createTerrainItem(ItemType type, const NxVec3& pos, float size)
	{
		return NxTerrainItem::createTerrainItem(type, pos, size);
	}
	
	NxArray<ItemPicked*>* getItemPicked()
	{
		return NxTerrainItem::getItemPicked();
	}
	
	void addScratchLine()	// 添加起跑线
	{
		NxTerrainItem::addScratchLine();
	}
	
	void releasePhysics()
	{
		m_NxPhysics->release();
	}
private:
	static NxAllVehicle* mAllVehicle;
	NxPhysicsSDK* m_NxPhysics;
	NxScene* m_NxScene;

	NxVec3* m_TerrainVerts;
	NxU32* m_TerrainFaces;
	NxMaterialIndex* m_TerrainMaterials;

	NxMaterialIndex m_MaterialIce;
	NxMaterialIndex m_MaterialRock;
	NxMaterialIndex m_MaterialMud;
	NxMaterialIndex m_MaterialGrass;
	NxMaterialIndex m_MaterialDefault;
};
