#include ".\NxTerrain.h"
#include <Stream.h>
#include "Define.h"
#include <Ogre.h>

NxAllVehicle* NxTerrain::mAllVehicle = NxAllVehicle::getSingleton();

NxTerrain::NxTerrain()
{
	m_NxPhysics = NULL;
	m_NxScene = NULL;

	m_TerrainVerts = 0;
	m_TerrainFaces = 0;
	m_TerrainMaterials = 0;

	m_MaterialDefault = 0;
}

NxTerrain::~NxTerrain(void)
{
}

// ��ʼ��Novodex
void NxTerrain::initNx()
{
	// Initialize PhysicsSDK
	m_NxPhysics = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, NULL);
	if(!m_NxPhysics)	
	{
		return;
	}
	NxInitCooking(NULL, NULL);

	m_NxPhysics->setParameter(NX_MIN_SEPARATION_FOR_PENALTY, -0.05f);
	//enable visualisation
	m_NxPhysics->setParameter(NX_VISUALIZATION_SCALE, 1.0f);

	m_NxPhysics->setParameter(NX_VISUALIZE_BODY_AXES, 1.0f);
	//m_NxPhysics->setParameter(NX_VISUALIZE_BODY_MASS_AXES, 1.0f);


	m_NxPhysics->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_COLLISION_AXES, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_CONTACT_POINT, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_CONTACT_NORMAL, 1.0f);

	m_NxPhysics->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_JOINT_WORLD_AXES, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_JOINT_ERROR, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_JOINT_FORCE, 1.0f);
	m_NxPhysics->setParameter(NX_VISUALIZE_JOINT_REDUCED, 1.0f);

	// Don't slow down jointed objects
	m_NxPhysics->setParameter(NX_ADAPTIVE_FORCE, 0.0f);

	//create some materials -- note that we reuse the same NxMaterial every time,
	//as it gets copied to the SDK with every setMaterial() call, not just referenced.

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = DEFAULT_GRAVITY;
	//sceneDesc.userContactReport = carContactReport;
	m_NxScene = m_NxPhysics->createScene(sceneDesc);

	//default material
	//defaultMaterialIndex = 0;
	NxMaterial* defaultMaterial = m_NxScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.8f);
	defaultMaterial->setDynamicFriction(0.8f);


	// Turn on all contact notifications:
	//m_NxScene->setActorGroupPairFlags(0, 0, NX_NOTIFY_ON_TOUCH);
}

/* faceIndex*3 / +1 / +2 ΪgTerrianVerts����,����ʱ,��ʱ��
ȡ1.0f-normal.y��lastSteepnex��ƽ��,�������ֵsteepnessȡ��ͬ�Ĳ���,
������initTerrain�ж���,ʹ�õ�ʱ������Ӧ��Index����,����steepness
ֻ��initTerrain��ʹ�ù�*/
NxReal NxTerrain::chooseTrigMaterial(NxU32 faceIndex, NxReal lastSteepness)
{
	NxVec3 & v0 = m_TerrainVerts[m_TerrainFaces[faceIndex * 3]];
	NxVec3 & v1 = m_TerrainVerts[m_TerrainFaces[faceIndex * 3 + 1]];
	NxVec3 & v2 = m_TerrainVerts[m_TerrainFaces[faceIndex * 3 + 2]];

	NxVec3 edge0 = v1 - v0;
	NxVec3 edge1 = v2 - v0;

	NxVec3 normal = edge0.cross(edge1);
	normal.normalize();
	NxReal steepness = 1.0f - normal.y;		// ȡ1.0f-normal.y��lastSteepnex��ƽ��
	steepness += lastSteepness;
	steepness /= 2.0f;

	if	(steepness > 0.25f)
	{
		m_TerrainMaterials[faceIndex] = m_MaterialIce;
	}
	else if (steepness > 0.2f)
	{
		m_TerrainMaterials[faceIndex] = m_MaterialRock;

	}
	else if (steepness > 0.1f)
	{
		m_TerrainMaterials[faceIndex] = m_MaterialMud;
	}
	else
		m_TerrainMaterials[faceIndex] = m_MaterialGrass;
	return steepness;
}

// ���������ͼ
void NxTerrain::createNxTerrain(void)
{
	NxMaterialDesc	m;

	NxMaterial * defaultMaterial = m_NxScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);	// ����,A coefficient of 0 makes the object bounce as little as possible
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);


	//terrain materials:

	// Ice
	m.restitution		= 0.0f;
	m.staticFriction	= 0.3f;
	m.dynamicFriction	= 0.3f;
	m_MaterialIce = m_NxScene->createMaterial(m)->getMaterialIndex();
	// Rock
	m.restitution		= 0.0f;
	m.staticFriction	= 1.2f;
	m.dynamicFriction	= 0.9f;
	m_MaterialRock = m_NxScene->createMaterial(m)->getMaterialIndex();
	// Mud
	m.restitution		= 0.0f;
	m.staticFriction	= 0.8f;
	m.dynamicFriction	= 0.4f;
	m_MaterialMud = m_NxScene->createMaterial(m)->getMaterialIndex();
	// Grass
	m.restitution		= 0.0f;
	m.staticFriction	= 0.4f;
	m.dynamicFriction	= 0.4f;
	m_MaterialGrass = m_NxScene->createMaterial(m)->getMaterialIndex();

	FILE* fin = fopen(NX_TERRAIN_FILE, "rb");
 	if (fin == NULL)
 	{
// //		pLog->logMessage("Cannot open terrain file in initTerrain()");
// 		OgreGuard( "initTerrain" );			// ����������ѹ���ջ
// 		OGRE_EXCEPT(888, "initTerrain(void)", "Cannot open terrain file" );	// �׳�һ���쳣
// 		OgreUnguard();				// ����ջ��Ԫ��
		return;
 	}
	char mark[4];
	fread(mark, 1, 4, fin);

 	if (!(mark[0] == 't' && mark[1] == 'e' && mark[2] == 'r' && mark[3] == 'r'))
 	{
// //		pLog->logMessage("Not a terr file!");
// 		OgreGuard( "initTerrain" );			// ����������ѹ���ջ
// 		OGRE_EXCEPT(888, "initTerrain(void)", "Not a terr file" );	// �׳�һ���쳣
// 		OgreUnguard();				// ����ջ��Ԫ��
		return;
 	}

	unsigned int nFaces;
	unsigned int nVerts;
	fread(&nVerts, 4, 1, fin);
	m_TerrainVerts = new NxVec3[nVerts];

	fread(m_TerrainVerts, sizeof(NxVec3), nVerts, fin);
	for (unsigned int i=0; i<nVerts; i++)	// �Ŵ�SCALE_TERRAIN��
	{
		m_TerrainVerts[i] *= SCALE_TERRAIN;
	}
	//fread(gTerrainNormals, sizeof(NxVec3), nVerts, fin);

	fread(&nFaces, 4, 1, fin);
	m_TerrainFaces = new NxU32[nFaces*3];
	fread(m_TerrainFaces, sizeof(NxU32), nFaces*3, fin);

	fclose(fin);

	//allocate terrain materials -- one for each face.
	m_TerrainMaterials = new NxMaterialIndex[nFaces];

	NxReal lastSteepness = 0.0f;
	for(NxU32 f=0;f<nFaces;f++)
	{
		//new: generate material indices for all the faces
		lastSteepness = chooseTrigMaterial(f, lastSteepness);
	}
	// Build vertex normals
	//g_TerrainNormals = new NxVec3[TERRAIN_NB_VERTS];
	//NxBuildSmoothNormals(TERRAIN_NB_FACES, TERRAIN_NB_VERTS, m_TerrainVerts, m_TerrainFaces, NULL, g_TerrainNormals, true);

	// Build physical model
	NxTriangleMeshDesc terrainDesc;
	terrainDesc.numVertices					= nVerts;
	terrainDesc.numTriangles				= nFaces;
	terrainDesc.pointStrideBytes			= sizeof(NxVec3);
	terrainDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	terrainDesc.points						= m_TerrainVerts;
	terrainDesc.triangles					= m_TerrainFaces;							
	terrainDesc.flags						= 0;
	//add the mesh material data:
	terrainDesc.materialIndexStride			= sizeof(NxMaterialIndex);
	terrainDesc.materialIndices				= m_TerrainMaterials;

	// To create a height field, set heightFieldVerticalAxis to NX_X, NX_Y or NX_Z, or leave it set to NX_NOT_HEIGHTFIELD for an arbitrary mesh
	// Default: NX_NOT_HEIGHTFIELD
	//terrainDesc.heightFieldVerticalAxis		= NX_Y;	// ����ע�͵�����Ϊ����һЩ��ֱ����,��ΪNX_Y��,��������ص���ײ���
	//terrainDesc.heightFieldVerticalExtent	= -1000.0f;

	NxTriangleMeshShapeDesc terrainShapeDesc;
	// This must be called at least once, before any cooking method is called (otherwise cooking fails).
	NxInitCooking();
	MemoryWriteBuffer buf;
	bool status = NxCookTriangleMesh(terrainDesc, buf);
	MemoryReadBuffer readBuffer(buf.data);
	terrainShapeDesc.meshData = m_NxPhysics->createTriangleMesh(readBuffer);
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&terrainShapeDesc);
	NxActor* NxTerr = m_NxScene->createActor(actorDesc);
	NxTerr->userData = (void*)0;
}

// �����������湦��
void NxTerrain::simulate(float time, int steering, int acceleration, bool handBrake)
{
	//INITLOG;

	// do some physics- limit time between 1/40s & 1/140s
	if (time < 1.0f/100.0f) 
	{
		time = 1.0f / 140.0f;
	}
	if (time > 1.0f/40.0f)
	{
		time = 1.0f / 40.0f;
	}
	//////////////////////////////////////

	if (mAllVehicle)
	{
		mAllVehicle->control(steering, acceleration, handBrake);
		mAllVehicle->updateAllVehicle(time);
	}

	m_NxScene->simulate(time);
	m_NxScene->flushStream();
	m_NxScene->fetchResults(NX_RIGID_BODY_FINISHED);
}

int NxTerrain::getOilAmount()
{
	return mAllVehicle->getOilAmount();
}

void NxTerrain::setOilAmount(int oilAmount)
{
	mAllVehicle->setOilAmount(oilAmount);
}

int NxTerrain::getLoopNum()
{
	return mAllVehicle->getLoopNum();
}

void NxTerrain::setLoopNum(int nLoop)
{
	mAllVehicle->setLoopNum(nLoop);
}

// ���Ƴ����˶�
void NxTerrain::control(int steering, int acceleration, bool handBrake)
{
	mAllVehicle->control(steering, acceleration, handBrake);
}

void NxTerrain::standUp()
{
	mAllVehicle->standUp();
}

void NxTerrain::setToBeginning(const NxVec3& pos)
{
	mAllVehicle->setToBeginning(pos);
}

void NxTerrain::addUser(const NxVec3& pos, VehicleInfo vinfo, std::string name)
{
	mAllVehicle->addVehicle(pos, vinfo, name, this->m_NxScene, this->m_NxPhysics);
}

AllVehicleInfoNx* NxTerrain::getAllVehicleInfo()
{
	return mAllVehicle->getVehicleInfo();
}

float NxTerrain::getVelocity()	// �õ������ٶ�
{
	return mAllVehicle->getUserVelocity();
}

// void NxTerrain::addTerrainItem(ItemType type, const NxVec3& pos, float size)
// {
// 	NxTerrainItem::createTerrainItem(type, pos, size, m_NxScene);
//}

int NxTerrain::userControl(const std::string& name)	// �����û�
{
	return mAllVehicle->setCurrentUser(name);
}

void NxTerrain::useSlipProp()	// ʹ�ô򻬵���
{
	this->mAllVehicle->setToSlip();
}

void NxTerrain::setMaxVelocity(float velocity)
{
	this->mAllVehicle->setMaxVelocity(velocity);
}

void NxTerrain::cancelSlipProp()	// ȥ���򻬵���
{
	this->mAllVehicle->cancleSlip();
}

void NxTerrain::useFlashProp()	// ʹ���������
{
}

void NxTerrain::cancelFlashProp()	// ȥ���������
{
}