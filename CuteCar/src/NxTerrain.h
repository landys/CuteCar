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
	void initNx();	// ��ʼ��Novodex
	NxReal chooseTrigMaterial(NxU32 faceIndex, NxReal lastSteepness);	// ���ݵ�����б��ѡ���������
	void createNxTerrain(void);	// ���������ͼ
	void simulate(float time, int steering, int acceleration, bool handBrake);	// �����������湦��
	
	void control(int steering, int acceleration, bool handBrake);	// ���Ƴ����˶�
	void standUp();	// �����˶���ʼ��
	void setToBeginning(const NxVec3& pos);
	void addUser(const NxVec3& pos, VehicleInfo vinfo, std::string name);	// ���һ���û�,����ʵ�ֵ������һ����
	//void addTerrainItem(ItemType type, const NxVec3& pos, float size);
	AllVehicleInfoNx* getAllVehicleInfo();	// �õ����г���������OGRE���Ƶ���Ϣ
	float getVelocity();	// �õ������ٶ�

	int getNumUser()	// �õ����û���Ŀ
	{
		return mAllVehicle->getNumUser();
	}
	int userControl(const std::string& name);	// �����û�
	int getOilAmount();
	void setOilAmount(int oilAmount);
	int getLoopNum();
	void setLoopNum(int nLoop);
	void useSlipProp();	// ʹ�ô򻬵���
	void cancelSlipProp();	// ȥ���򻬵���
	void useFlashProp();	// ʹ���������
	void cancelFlashProp();	// ȥ���������
	void setMaxVelocity(float velocity);
	NxScene* getNxScene()	// �õ�������
	{
		return m_NxScene;
	}

	// ���߷�װ
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
	
	void addScratchLine()	// ���������
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
