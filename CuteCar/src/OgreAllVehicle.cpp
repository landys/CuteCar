#include ".\ogreallvehicle.h"

/**************************************************************/
/* OgreAllVehicle 
/**************************************************************/

OgreAllVehicle* OgreAllVehicle::mSingleton = NULL;

OgreAllVehicle::OgreAllVehicle(void)
{
//	miCurrentVehicle = -1;
	mAllVehicle.clear();
	mSingleton = NULL;
}

OgreAllVehicle::~OgreAllVehicle(void)
{

}

OgreAllVehicle* OgreAllVehicle::getSingleton()	// 得到唯一的车辆管理的对象
{
	if (mSingleton == NULL)
	{
		mSingleton = new OgreAllVehicle();
	}
	return mSingleton;
}

OgreVehicle* OgreAllVehicle::addVehicle(Ogre::SceneNode* ogreRoot, std::string name, Ogre::Vector3 pos)
{
	OgreVehicle* vehicle = OgreVehicle::createOgreVehicle(ogreRoot, name, pos);
	if (vehicle)
	{
		mAllVehicle.push_back(vehicle);
	}
	
	return vehicle;
}

void OgreAllVehicle::updateAllVehicle(AllVehicleInfoOgre* allvehicle)
{
	INITLOG;
	unsigned int n = mAllVehicle.size();
	for (unsigned int i=0; i<n; i++)
	{
		OgreVehicle* ve = this->getUserVehicle(allvehicle[i].name);
		ve->updateVehicle(allvehicle[i]);
	}
}

OgreVehicle* OgreAllVehicle::getUserVehicle(const std::string& name)	//得到用户名为name的用户的车
{
	unsigned int n = mAllVehicle.size();
	for (unsigned int i=0; i<n; i++)
	{
		if (mAllVehicle[i]->getVehicleName() == name)
		{
			return mAllVehicle[i];
		}
	}
	return NULL;
}

OgreVehicle* OgreAllVehicle::RemoveVehicleCam()	// 使当前视角的车释放照相机,并且返回其下一辆车
{
	unsigned int n = mAllVehicle.size();
	if (n == 0)
	{
		return NULL;
	}

	for (unsigned int i=0; i<n; i++)
	{
		if (mAllVehicle[i]->hasCamera())
		{
			mAllVehicle[i]->removeCamera();
			if (++i >= n)
			{
				i = 0;
			}
			return mAllVehicle[i];
		}
	}

	return mAllVehicle[0];
}

// void OgreAllVehicle::setToBeginning(const std::string& name)
// {
// 	unsigned int n = mAllVehicle.size();
// 	for (unsigned int i=0; i<n; i++)
// 	{
// 		if (mAllVehicle[i]->getVehicleName() == name)
// 		{
// 			mAllVehicle[i]->setToBeginning();
// 		}
// 	}
// 	return NULL;
//}

/**************************************************************/
/* OgreVehicle 
/**************************************************************/
OgreVehicle::OgreVehicle(void)
{
	mParNode = NULL;
	mNode = NULL;
	mCam = NULL;
	for (int i=0; i<4; i++)
	{
		mWheels[i] = NULL;
	}
	mName = "";
	miCurItem = 0;
}

OgreVehicle::~OgreVehicle(void)
{

}

OgreVehicle* OgreVehicle::createOgreVehicle(Ogre::SceneNode* parNode, std::string name, Ogre::Vector3 pos)
{
	if (parNode)
	{
		OgreVehicle* vehicle = new OgreVehicle();
		vehicle->mName = name;
		vehicle->mParNode = parNode;
		Ogre::SceneManager* sceneMgr = vehicle->mParNode->getCreator();
		Ogre::String nodeName = "User_" + name;
		vehicle->mNode = vehicle->mParNode->createChildSceneNode(nodeName);
		Ogre::SceneNode* pNodeBody = vehicle->mNode->createChildSceneNode("Body" + nodeName);
		Ogre::SceneNode* pNodeCam = vehicle->mNode->createChildSceneNode("Cam" + nodeName);
		Ogre::Entity* eBody = sceneMgr->createEntity("eBody"+nodeName, OGRE_VEHICLE_FILE);
		pNodeBody->attachObject(eBody);
		pNodeBody->setScale(0.0002*SCALE_TERRAIN, 0.0002*SCALE_TERRAIN, 0.0002*SCALE_TERRAIN);
		vehicle->mNode->setPosition(pos);

		vehicle->mWheels[0] = OgreWheel::createOgreWheel(pNodeBody, Ogre::Vector3(1.02f, -0.3622f, 1.26));
		vehicle->mWheels[1] = OgreWheel::createOgreWheel(pNodeBody, Ogre::Vector3(1.12f, -0.3622f,-1.54));
		vehicle->mWheels[2] = OgreWheel::createOgreWheel(pNodeBody, Ogre::Vector3(-1.02f, -0.3622f, 1.26));
		vehicle->mWheels[3] = OgreWheel::createOgreWheel(pNodeBody, Ogre::Vector3(-1.12f, -0.3622f,-1.54));
		
		return vehicle;
	}
	return NULL;
}

void OgreVehicle::updateVehicle(const AllVehicleInfoOgre& allv)
{
	//INITLOG;
	//PRINTLOG(allv.vepos.y);
	Ogre::SceneNode* pNode = (Ogre::SceneNode*)(mNode->getChild("Body" + mNode->getName()));
	pNode->setOrientation(allv.verot);
	mNode->setPosition(allv.vepos - mParNode->getWorldPosition());

	if (allv.campos != Ogre::Vector3(0, 0, 0))
	{
		Ogre::SceneNode* pCamNode = (Ogre::SceneNode*)(mNode->getChild("Cam" + mNode->getName()));

		pCamNode->setOrientation(allv.camrot);
		pCamNode->setPosition(allv.campos);
	}
	
	mWheels[0]->updateWheel(allv.angle0);
	mWheels[1]->updateWheel(allv.angle1);
	mWheels[2]->updateWheel(allv.angle2);
	mWheels[3]->updateWheel(allv.angle3);
}

std::string OgreVehicle::getVehicleName()
{
	return mName;
}

void OgreVehicle::setToBeginning()
{
	this->miCurItem = 0;
	this->mItems.clear();
}

void OgreVehicle::attachCamera(Ogre::Camera* cam)
{
	if (cam && !mCam)
	{
		Ogre::SceneNode* pNodeCam = (Ogre::SceneNode*)(mNode->getChild("Cam"+mNode->getName()));
		pNodeCam->attachObject(cam);
		mCam = cam;
	}
}

void OgreVehicle::setCamera(const Ogre::Vector3& repos, const Ogre::Quaternion& reorien)
{
	if (mCam)
	{
		mCam->setPosition(0, 0, 0);
		Ogre::SceneNode* pNodeCam = (Ogre::SceneNode*)(mNode->getChild("Cam"+mNode->getName()));
		Ogre::SceneNode* pNodeBody = (Ogre::SceneNode*)(mNode->getChild("Body"+mNode->getName()));
		pNodeCam->setPosition(repos);
		//m_pCam->lookAt(pNodeBody->getWorldPosition() + Ogre::Vector3(0, 0, -20));
		mCam->setOrientation(reorien);
	}
}

void OgreVehicle::removeCamera()
{
	if (mCam)
	{
		Ogre::SceneNode* pNodeCam = (Ogre::SceneNode*)(mNode->getChild("Cam"+mNode->getName()));
		if (pNodeCam)
		{
			pNodeCam->detachAllObjects();
		}
		
		mCam = NULL;
	}
}

bool OgreVehicle::hasCamera()	// 是否拥有照相机
{
	return (mCam != NULL);
}

bool OgreVehicle::addVehicleItem(ItemType type)
{
	if (mItems.size() >= VEHICLE_MAX_ITEMS || !(type == ITEM_TYPE_BIGOIL || type == ITEM_TYPE_MIDOIL
		|| type == ITEM_TYPE_SMALLOIL || type == ITEM_TYPE_SPEEDUP))
	{
		return false;
	}

	mItems.push_back(type);
	return true;
}

std::vector<ItemType>* OgreVehicle::getAllItem()
{
	return &mItems;
}

ItemType OgreVehicle::getCurrentItem()	// 得到当前道具
{
	if (miCurItem >= 0 && miCurItem < mItems.size())
	{
		return mItems[miCurItem];
	}
	else if (mItems.size() > 0)
	{
		miCurItem = 0;
		return mItems[miCurItem];
	}
	return ITEM_NONE;
}

ItemType OgreVehicle::removeCurrentItem()	// 返回当前道具后移除,并移到下一个道具位置处
{
	ItemType it = ITEM_NONE;
	if (miCurItem >= 0 && miCurItem < mItems.size())
	{
		it = mItems[miCurItem];
	}
	else if (mItems.size() > 0)
	{
		miCurItem = 0;
		it = mItems[miCurItem];
	}
	if (++miCurItem >= mItems.size())
	{
		miCurItem = 0;
	}
	return it;
}

/******************************************************************/
/* OgreWheel
/******************************************************************/

int OgreWheel::mWheelCount = 0;
OgreWheel::OgreWheel(void)
{

}

OgreWheel::~OgreWheel(void)
{

}

OgreWheel* OgreWheel::createOgreWheel(Ogre::SceneNode* parNode, Ogre::Vector3 pos)
{
	if (parNode)
	{
		OgreWheel* wheel = new OgreWheel();
		wheel->mParNode = parNode;
		wheel->mNode = wheel->mParNode->createChildSceneNode("Wheel" + Ogre::StringConverter::toString(++mWheelCount));
		Ogre::SceneManager* smgr = wheel->mParNode->getCreator();

		Ogre::Entity* eBody = smgr->createEntity("eBodyWheel" + Ogre::StringConverter::toString(mWheelCount), OGRE_WHEEL_FILE);
		wheel->mNode->attachObject(eBody);

		wheel->mNode->setPosition(pos/0.0006);
		return wheel;
	}
	return NULL;
}

void OgreWheel::updateWheel(const float angle)
{
	// Render change angle
	Ogre::Quaternion qq;
	qq.FromAngleAxis(Ogre::Radian(-angle)*5, Ogre::Vector3(0, 1, 0));
	mNode->setOrientation(qq);
}

/******************************************************************/
/* OgreTerrainItem
/******************************************************************/
OgreTerrainItem::OgreTerrainItem(Ogre::SceneNode* parNode, ItemType type, int id) :
	mParNode(parNode), mItemType(type), mId(id), mNode(NULL)
{
}

OgreTerrainItem::~OgreTerrainItem(void)
{

}

OgreTerrainItem* OgreTerrainItem::createOgreTerrainItem(Ogre::SceneNode* parNode, const Ogre::Vector3& pos, ItemType type, int id)
{
	if (parNode)
	{
		float scale = 0.06;
		Ogre::String fileName = "";
		switch (type)
		{
		case ITEM_TYPE_SLIP:
			fileName = OGRE_ITEM_SLIP_FILE;
			scale = 0.18;
			break;
		case ITEM_TYPE_BIGOIL:
			fileName = OGRE_ITEM_BIGOIL_FILE;
			scale = 0.24;
			break;
		case ITEM_TYPE_MIDOIL:
			fileName = OGRE_ITEM_MIDOIL_FILE;
			scale = 0.20;
			break;
		case ITEM_TYPE_SMALLOIL:
			fileName = OGRE_ITEM_SMALLOIL_FILE;
			scale = 0.16;
			break;
		case ITEM_TYPE_SPEEDUP:
			fileName = OGRE_ITEM_SPEEDUP_FILE;
			scale = 0.09;
			break;
		case ITEM_TYPE_BLOCK:
			fileName = OGRE_ITEM_BLOCK_FILE;
			scale = 0.08;
			break;
		case ITEM_TYPE_ILL:
			fileName = OGRE_ITEM_ILL_FILE;
			scale = 0.08;
			break;
		}
		OgreTerrainItem* oti = new OgreTerrainItem(parNode, type, id);
		Ogre::SceneManager* sceneMgr = oti->mParNode->getCreator();
		Ogre::String nodeName = "Item_" + Ogre::StringConverter::toString(id);
		oti->mNode = oti->mParNode->createChildSceneNode(nodeName);
		Ogre::Entity* eBody = sceneMgr->createEntity("eBody"+nodeName, fileName);
		oti->mNode->attachObject(eBody);
		oti->mNode->setScale(scale, scale, scale);
		//oti->mNode->setScale(0.02*SCALE_TERRAIN, 0.02*SCALE_TERRAIN, 0.02*SCALE_TERRAIN);
		oti->mNode->setPosition(pos);

		return oti;
	}
	return NULL;
}

bool OgreTerrainItem::removeTerrainItem(int id)	// 移除道具,是这个id即成功返回true	
{
	if (mId == id)
	{
		mNode->removeAndDestroyAllChildren();
		mParNode->removeChild(mNode);
		return true;
	}
	return false;
}

bool OgreTerrainItem::turnTerrainItem(int id, float angle)	// 转y轴即竖直方向旋转道具,是这个id即可功返回true  
{
	if (mId == id)
	{
		Ogre::Quaternion rot;
		mNode->yaw(Ogre::Radian(angle));
		return true;
	}
	return false;
}

int OgreTerrainItem::getTerrainItemId()	// 得到该道具的ID
{
	return mId;
}
