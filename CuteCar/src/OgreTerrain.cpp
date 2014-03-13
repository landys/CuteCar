#include ".\ogreterrain.h"

OgreTerrain::OgreTerrain(Ogre::SceneManager* sceneMgr)
{
	mOgreSceneMgr = sceneMgr;
	mOgreRoot = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTerrainItems.clear();
}

OgreTerrain::~OgreTerrain(void)
{
}

void OgreTerrain::createOgreTerrain(void)	// Ogre��ͼ����
{
	mOgreSceneMgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));
	mOgreSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);	// ���

	Ogre::Entity* terr = mOgreSceneMgr->createEntity("terrain", OGRE_TERRAIN_FILE);	// ����
	Ogre::SceneNode* tNode = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode();
	tNode->attachObject(terr);
	tNode->setScale(SCALE_TERRAIN, SCALE_TERRAIN, SCALE_TERRAIN);	// ���ű���
}

void OgreTerrain::addUser(std::string name, Ogre::Vector3 pos)
{
	OgreAllVehicle* allv = OgreAllVehicle::getSingleton();
	allv->addVehicle(mOgreRoot, name, pos);
}

void OgreTerrain::updateAllOgre(AllVehicleInfoOgre* allvehicle)	//**************** temp
{
	INITLOG;
	//pLog->logMessage("void OgreTerrain::updateAllOgre(AllVehicleInfoOgre* allvehicle)");
	OgreAllVehicle* allv = OgreAllVehicle::getSingleton();
	allv->updateAllVehicle(allvehicle);
}

int OgreTerrain::userCamera(Ogre::Camera* cam, const std::string& name)
{
	INITLOG;
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->getUserVehicle(name);
	if (vehicle != NULL)
	{
		PRINTLOG(1);
		vehicle->removeCamera();
		PRINTLOG(2);
		vehicle->attachCamera(cam);
		PRINTLOG(3);
		vehicle->setCamera(Ogre::Vector3(0, 13*SCALE_TERRAIN, 30*SCALE_TERRAIN), Ogre::Quaternion(0.701396, -0.0896904, -0.701395, -0.0896904));//Vector3(0.f, 0.f, -20.f));
		PRINTLOG(4);
		return 1;
	}

	return 0;
}

void OgreTerrain::wholeCamera(Ogre::Camera* cam)	// ȫ���ӽ�
{
	OgreAllVehicle::getSingleton()->RemoveVehicleCam();
	cam->setPosition(Ogre::Vector3(70*SCALE_TERRAIN, 72*SCALE_TERRAIN, 90*SCALE_TERRAIN));
	cam->setOrientation(Ogre::Quaternion(0.981956, -0.189108, 0, 0));
}

int OgreTerrain::switchCamera(Ogre::Camera* cam)	// ת���ӽǵ���һ���û�
{
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->RemoveVehicleCam();
	if (vehicle != NULL)
	{	
		vehicle->attachCamera(cam);
		vehicle->setCamera(Ogre::Vector3(0, 13*SCALE_TERRAIN, 30*SCALE_TERRAIN), Ogre::Quaternion(0.701396, -0.0896904, -0.701395, -0.0896904));//Vector3(0.f, 0.f, -20.f));

		return 1;
	}
	return 0;
}

void OgreTerrain::addVehicleItem(ItemType type, const std::string& name)	// �񵽵��ߺ���ӵ������б���
{
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->getUserVehicle(name);
	vehicle->addVehicleItem(type);
}

void OgreTerrain::addTerrainItem(const Ogre::Vector3& pos, ItemType type, int id)	// ��ӵ��ߵ�������
{
	OgreTerrainItem* oti = OgreTerrainItem::createOgreTerrainItem(mOgreRoot, pos, type, id);
	mTerrainItems.push_back(oti);
}

void OgreTerrain::removeTerrainItem(int id)	// ɾ�������е���
{
	for (std::vector<OgreTerrainItem*>::iterator i=mTerrainItems.begin(); i!=mTerrainItems.end(); i++)
	{
		if ((*i)->removeTerrainItem(id))
		{
			mTerrainItems.erase(i);
			return;
		}
	}
}

void OgreTerrain::turnAllTerrainItem(float angle)	// תy�ἴ��ֱ������ת���е���
{
	for (std::vector<OgreTerrainItem*>::iterator i=mTerrainItems.begin(); i!=mTerrainItems.end(); i++)
	{
		(*i)->turnTerrainItem((*i)->getTerrainItemId(), angle);
	}
}

ItemType OgreTerrain::getCurrentVehicleItem(const std::string& name)	// �õ���ǰ����
{
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->getUserVehicle(name);
	return vehicle->getCurrentItem();
}

std::vector<ItemType>* OgreTerrain::getAllItem(const std::string& name)
{
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->getUserVehicle(name);
	return vehicle->getAllItem();
}

ItemType OgreTerrain::removeCurrentVehicleItem(const std::string& name)	// ���ص�ǰ���ߺ��Ƴ�,���Ƶ���һ������λ�ô�
{
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->getUserVehicle(name);
	return vehicle->removeCurrentItem();
}

void OgreTerrain::addScratchLine()	// ���������
{
	Ogre::Entity* eLine = mOgreSceneMgr->createEntity("scratchLine", OGRE_SCRATCH_LINE_FILE);	// ����
	Ogre::SceneNode* tNode = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode();
	tNode->attachObject(eLine);
	tNode->setPosition(24.6*SCALE_TERRAIN, 0.59*SCALE_TERRAIN, -69.52*SCALE_TERRAIN);
	tNode->yaw(Ogre::Radian(Ogre::Math::PI / 2));
	tNode->setScale(SCALE_TERRAIN*0.25, SCALE_TERRAIN*0.25, SCALE_TERRAIN*0.26);	// ���ű���
}

void OgreTerrain::setToBeginning(const std::string& name)	// �ָ�ֵ���մ���ʱ
{
	OgreVehicle* vehicle = OgreAllVehicle::getSingleton()->getUserVehicle(name);
	vehicle->setToBeginning();
}


