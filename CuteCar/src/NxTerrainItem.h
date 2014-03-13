#pragma once

#include <NxPhysics.h>
#include <string>
#include "Define.h"

class ItemPicked
{
public:
	int id;
	ItemType type;
	std::string name;
};

class NxTerrainItem
{
public:
	static void initTerrainItem(NxScene* scene);//, NxArray<ItemPicked*>* itemPicked);
	static NxTerrainItem* createTerrainItem(ItemType type, const NxVec3& pos, float size);
	static void addScratchLine();	// 添加起跑线
	static NxArray<ItemPicked*>* getItemPicked(bool isAll=false);
	int getItemId()
	{
		return mId;
	}
	ItemType getItemType()
	{
		return mItemType;
	}

protected:
	NxTerrainItem(ItemType itemType);
	virtual ~NxTerrainItem(void);
private:
	int mId;
	ItemType mItemType;
	class ItemTrigger;	// 继承NxUserTriggerReport类
	static ItemTrigger* mItemTrigger;
	static NxArray<ItemPicked*> mItemPicked;
	static NxScene* mScene;
	static int mItemCount;
};
