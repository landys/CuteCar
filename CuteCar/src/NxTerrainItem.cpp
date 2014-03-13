#include ".\NxTerrainItem.h"
#include "NxVehicle.h"
#include <NxUserContactReport.h> 


NxArray<ItemPicked*> NxTerrainItem::mItemPicked;
NxScene* NxTerrainItem::mScene = NULL;
int NxTerrainItem::mItemCount = 0;

class NxTerrainItem::ItemTrigger : public NxUserTriggerReport
{
public:
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		NxActor& actor = triggerShape.getActor();
		NxTerrainItem* ti = (NxTerrainItem*)(actor.userData);
		try
		{
			NxVehicle* ve = (NxVehicle*)(otherShape.getActor().userData);
			if(status & NX_TRIGGER_ON_ENTER)
 			{
				// 车子过起跑线共有7种状态(进入的方向不同也不一样),在起跑线外(1),在一个半起跑线内(2/7, 4/5),在两个半起跑线上(3/6)
				if (ti->getItemType() == ITEM_SCRATCH_LINE_IN)	// 进入起跑线
				{
					if (ve->mStatusScratch == 1)
					{
						ve->mStatusScratch = 2;
					}
					else if (ve->mStatusScratch == 4)
					{
						ve->mStatusScratch = 3;
					}
					else if (ve->mStatusScratch == 5)
					{
						ve->mStatusScratch = 6;
					}

				}
				else if (ti->getItemType() == ITEM_SCRATCH_LINE_OUT)	// 出起跑线
				{
					if (ve->mStatusScratch == 1)
					{
						ve->mStatusScratch = 5;
					}
					else if (ve->mStatusScratch == 2)
					{
						ve->mStatusScratch = 3;
					}
					else if (ve->mStatusScratch == 7)
					{
						ve->mStatusScratch = 6;
					}
				}
				else	// 真正的道具
				{
					ItemPicked* ip = new ItemPicked();
					ip->id = ti->getItemId();
					ip->name = ve->getVehicleName();
					ip->type = ti->getItemType();
					// A body entered the trigger area for the first time
					mItemPicked.push_back(ip);
					mScene->releaseActor(actor);
					delete ti;
				}
 			}

	 		if(status & NX_TRIGGER_ON_LEAVE)
	 		{
				if (ti->getItemType() == ITEM_SCRATCH_LINE_IN)	// 进入起跑线
				{
					if (ve->mStatusScratch == 7)
					{
						ve->mStatusScratch = 1;
						ve->setLoopNum(ve->getLoopNum() - 1);
					}
					else if (ve->mStatusScratch == 2)
					{
						ve->mStatusScratch = 1;
					}
					else if (ve->mStatusScratch == 6)
					{
						ve->mStatusScratch = 5;
					}
					else if (ve->mStatusScratch == 3)
					{
						ve->mStatusScratch = 4;
					}
				}
				else if (ti->getItemType() == ITEM_SCRATCH_LINE_OUT)	// 出起跑线
				{
					if (ve->mStatusScratch == 4)
					{
						ve->mStatusScratch = 1;
						ve->setLoopNum(ve->getLoopNum() + 1);
					}
					else if (ve->mStatusScratch == 5)
					{
						ve->mStatusScratch = 1;
					}
					else if (ve->mStatusScratch == 3)
					{
						ve->mStatusScratch = 2;
					}
					else if (ve->mStatusScratch == 6)
					{
						ve->mStatusScratch = 7;
					}
				}
	 		}
		}
		catch (...)
		{
		}
 		
	}
};

NxTerrainItem::ItemTrigger* NxTerrainItem::mItemTrigger = new NxTerrainItem::ItemTrigger();

NxTerrainItem::NxTerrainItem(ItemType itemType) : mItemType(itemType)
{
	mItemPicked.clear();
}

NxTerrainItem::~NxTerrainItem(void)
{
}

void NxTerrainItem::initTerrainItem(NxScene* scene)
{
	mScene = scene;
	mScene->setUserTriggerReport(mItemTrigger);
}

// Create a static trigger
NxTerrainItem* NxTerrainItem::createTerrainItem(ItemType type, const NxVec3& pos, float size)
{
	// Our trigger is a cube
	NxTerrainItem* ti = new NxTerrainItem(type);
	//NxBodyDesc triggerBody;
	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions = NxVec3(size, size, size);
	BoxDesc.shapeFlags |= NX_TRIGGER_ENABLE;	// 其实只要这里设置了就可以实现trigger效果了

	NxActorDesc ActorDesc;

	ActorDesc.shapes.pushBack(&BoxDesc);
	ActorDesc.globalPose.t = pos;

	ActorDesc.userData = (void*)ti;

	if (!ActorDesc.isValid()) 
	{
//		printf("Invalid ActorDesc\n");
		return NULL;
	}
	NxActor* actor = mScene->createActor(ActorDesc);	// This is just a quick-and-dirty way to identify the trigger for rendering
	
	if (actor == NULL)
	{
		return NULL;
	}

	ti->mId = mItemCount++;
	//mScene->setUserTriggerReport(mItemTrigger);
	return ti;
}

void NxTerrainItem::addScratchLine()	// 添加起跑线
{
	// Our scratchLine is a cube
	// in half
	NxTerrainItem* ti1 = new NxTerrainItem(ITEM_SCRATCH_LINE_IN);
	NxBoxShapeDesc BoxDesc1;
	BoxDesc1.dimensions = NxVec3(6.7956*SCALE_TERRAIN, 5*SCALE_TERRAIN, 0.2*SCALE_TERRAIN);
	BoxDesc1.shapeFlags |= NX_TRIGGER_ENABLE;	// 其实只要这里设置了就可以实现trigger效果了

	NxActorDesc ActorDesc1;

	ActorDesc1.shapes.pushBack(&BoxDesc1);
	ActorDesc1.globalPose.t = NxVec3(24.6*SCALE_TERRAIN, 5*SCALE_TERRAIN, -69.16*SCALE_TERRAIN);

	ActorDesc1.userData = (void*)ti1;

	if (!ActorDesc1.isValid()) 
	{
		return;
	}
	NxActor* actor1 = mScene->createActor(ActorDesc1);	// This is just a quick-and-dirty way to identify the trigger for rendering
	NX_ASSERT(actor1 != NULL);

	// out half
	NxTerrainItem* ti2 = new NxTerrainItem(ITEM_SCRATCH_LINE_OUT);
	NxBoxShapeDesc BoxDesc2;
	BoxDesc2.dimensions = NxVec3(6.7956*SCALE_TERRAIN, 5*SCALE_TERRAIN, 0.2*SCALE_TERRAIN);
	BoxDesc2.shapeFlags |= NX_TRIGGER_ENABLE;	// 其实只要这里设置了就可以实现trigger效果了

	NxActorDesc ActorDesc2;

	ActorDesc2.shapes.pushBack(&BoxDesc2);
	ActorDesc2.globalPose.t = NxVec3(24.6*SCALE_TERRAIN, 0*SCALE_TERRAIN, -69.88*SCALE_TERRAIN);

	ActorDesc2.userData = (void*)ti2;

	if (!ActorDesc2.isValid()) 
	{
		return;
	}
	NxActor* actor2 = mScene->createActor(ActorDesc2);	// This is just a quick-and-dirty way to identify the trigger for rendering
	NX_ASSERT(actor2 != NULL);

// 	mScene->setUserTriggerReport(mItemTrigger);
}

NxArray<ItemPicked*>* NxTerrainItem::getItemPicked(bool isAll)
{
	if (isAll)
	{

	}
	return &mItemPicked;
}

