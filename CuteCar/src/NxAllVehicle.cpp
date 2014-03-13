#include ".\nxallvehicle.h"
#include <Stream.h>
#include <NxCooking.h>
#include "Define.h"
#include <Ogre.h>

NxAllVehicle* NxAllVehicle::mSingleton = 0;

NxAllVehicle::NxAllVehicle(void)
{
	mCountDead = 0;
	miUserVehicle = -1;
	mAllVehicle.clear();
	mIsLive.clear();
	mSizeAllInfoNx = 0;
	mAllInfoNx = NULL;
}

NxAllVehicle::~NxAllVehicle(void)
{
}

// 得到唯一的车辆管理的对象
NxAllVehicle* NxAllVehicle::getSingleton()
{
	if (mSingleton == 0)
	{
		mSingleton = new NxAllVehicle();
	}
	return mSingleton;
}

// 增加一辆车
NxVehicle* NxAllVehicle::addVehicle(const NxVec3& pos, VehicleInfo vinfo, std::string name, NxScene* nxScene, NxPhysicsSDK* nxPhysics)
{
	NxVehicleDesc vehicleDesc;
	NxBoxShapeDesc boxShapes[2];
	NxConvexShapeDesc carShape[2];

	NxArray<NxVec3> points;
	NxArray<NxVec3> points2;
	NxReal halfWidth = vinfo.width / 2;//1.1529f;
	NxReal halfLength = vinfo.length / 2;//2.5278f;
	NxReal halfHeight = vinfo.height / 2; //0.6027;

	points.pushBack().set(halfLength,-halfHeight * 0.1f, 0);
	points.pushBack().set(halfLength * 0.7f, 0, 0);
	points.pushBack().set(0.2f * halfLength, halfHeight * 0.2f, 0);
	points.pushBack().set(-halfLength, halfHeight * 0.2f, 0);
	points.pushBack().set(0.1*halfLength, halfHeight * 0.2f, halfWidth * 0.9f);
	points.pushBack().set(0.1*halfLength, halfHeight * 0.2f,-halfWidth * 0.9f);
	points.pushBack().set(-0.8*halfLength, halfHeight * 0.2f, halfWidth * 0.9f);
	points.pushBack().set(-0.8*halfLength, halfHeight * 0.2f,-halfWidth * 0.9f);

	points.pushBack().set(halfLength * 0.9f,-halfHeight * 0.25f, halfWidth * 0.8f);
	points.pushBack().set(halfLength * 0.9f,-halfHeight * 0.25f,-halfWidth * 0.8f);
	points.pushBack().set(0,-halfHeight * 0.2f, halfWidth);
	points.pushBack().set(0,-halfHeight * 0.2f,-halfWidth);
	points.pushBack().set(-halfLength * 0.9f,-halfHeight * 0.2f, halfWidth * 0.9f);
	points.pushBack().set(-halfLength * 0.9f,-halfHeight * 0.2f,-halfWidth * 0.9f);

	points.pushBack().set(halfLength * 0.8f, -halfHeight, halfWidth * 0.79f);
	points.pushBack().set(halfLength * 0.8f, -halfHeight,-halfWidth * 0.79f);
	points.pushBack().set(-halfLength * 0.8f, -halfHeight, halfWidth * 0.79f);
	points.pushBack().set(-halfLength * 0.8f, -halfHeight,-halfWidth * 0.79f);

	for(NxU32 i = 2; i < 8; i++)
	{
		points2.pushBack(points[i]);
	}

	points2.pushBack().set(-0.5*halfLength, halfHeight*0.8f, halfWidth*0.7f);
	points2.pushBack().set(-0.5*halfLength, halfHeight*0.8f,-halfWidth*0.7f);
	points2.pushBack().set(-0.7*halfLength, halfHeight*0.7f, halfWidth*0.7f);
	points2.pushBack().set(-0.7*halfLength, halfHeight*0.7f,-halfWidth*0.7f);


	static NxConvexMeshDesc convexMesh;
	convexMesh.numVertices = points.size();
	convexMesh.points = &(points[0].x);
	convexMesh.pointStrideBytes = sizeof(NxVec3);
	convexMesh.flags |= NX_CF_COMPUTE_CONVEX|NX_CF_USE_LEGACY_COOKER;

	MemoryWriteBuffer buf;
	bool status = NxCookConvexMesh(convexMesh, buf);
	if(status)
	{
		carShape[0].meshData = nxPhysics->createConvexMesh(MemoryReadBuffer(buf.data));
		vehicleDesc.carShapes.pushBack(&carShape[0]);
	}

	static NxConvexMeshDesc convexMesh2;
	convexMesh2.numVertices = points2.size();
	convexMesh2.points = (&points2[0].x);
	convexMesh2.pointStrideBytes = sizeof(NxVec3);
	convexMesh2.flags = NX_CF_COMPUTE_CONVEX|NX_CF_USE_LEGACY_COOKER;

	MemoryWriteBuffer buf2;
	status = NxCookConvexMesh(convexMesh2, buf2);
	if(status)
	{
		carShape[1].meshData = nxPhysics->createConvexMesh(MemoryReadBuffer(buf2.data));
		vehicleDesc.carShapes.pushBack(&carShape[1]);
	}

	vehicleDesc.position				= pos;
	vehicleDesc.mass					= vinfo.mass;//1200;//monsterTruck ? 12000 : 
	vehicleDesc.digitalSteeringDelta	= vinfo.steerablity;//0.04f;
	vehicleDesc.steeringMaxAngle		= vinfo.maxSteeringAngle;	//30.f;
	vehicleDesc.motorForce				= vinfo.maxAcceleraion * vinfo.mass;//3500.f;//monsterTruck?180.f:
	
	NxVehicleMotorDesc motorDesc;
	NxVehicleGearDesc gearDesc;
	NxReal wheelRadius = 0.4f;

	vehicleDesc.maxVelocity = vinfo.maxVelocity;	//80.f;//(monsterTruck)?40.f:80.f;
	motorDesc.setToCorvette();
	vehicleDesc.motorDesc = &motorDesc;
	gearDesc.setToCorvette();
	vehicleDesc.gearDesc = &gearDesc;
	vehicleDesc.differentialRatio = 3.42f;

	wheelRadius = 0.3622f;
	vehicleDesc.centerOfMass.set(0,-0.7f,0);


	NxWheelDesc wheelDesc[4];
	for(NxU32 i=0;i<4;i++)
	{
		wheelDesc[i].wheelApproximation = 10;
		//wheelDesc[i].wheelFlags |= NX_WF_BUILD_LOWER_HALF;
		wheelDesc[i].wheelRadius = wheelRadius;//(monsterTruck)?wheelRadius*3.f:wheelRadius;
		wheelDesc[i].wheelWidth = 0.1923f;//(monsterTruck)?0.3f:0.1923f;
		wheelDesc[i].wheelSuspension = 0.2f;//(monsterTruck)?0.6f:0.2f;
		wheelDesc[i].springRestitution = 7000;//monsterTruck?(crovette?5000:4000):7000;
		wheelDesc[i].springDamping = 800;
		wheelDesc[i].springBias = 0.0f;	// 设为1.0后居然会出错!!!!!!!!
		//wheelDesc[i].maxHandBraking = 1.f; //monsterTruck?0.5f:1.f;
		wheelDesc[i].inverseWheelMass = 4.0f / vinfo.maxAcceleraion;	// 换算成动力
		wheelDesc[i].frictionToFront = 1.f;
		wheelDesc[i].frictionToSide = 2.f;
		
		vehicleDesc.carWheels.pushBack(&wheelDesc[i]);
	}

	NxReal heightPos = -0.3622f;	//(monsterTruck)?1.f:
	wheelDesc[0].position.set( 1.02f, heightPos, 1.26);
	wheelDesc[1].position.set( 1.12f, heightPos,-1.54);
	wheelDesc[2].position.set(-1.02f, heightPos, 1.26);
	wheelDesc[3].position.set(-1.12f, heightPos,-1.54);
	NxU32 flags = NX_WF_BUILD_LOWER_HALF;

	wheelDesc[0].wheelFlags |= ((vinfo.driven==FrontDriven)?NX_WF_ACCELERATED:0) | NX_WF_STEERABLE_INPUT | flags;
	wheelDesc[1].wheelFlags |= ((vinfo.driven==FrontDriven)?NX_WF_ACCELERATED:0) | NX_WF_STEERABLE_INPUT | flags;
	wheelDesc[2].wheelFlags |= ((vinfo.driven==BackDriven)?NX_WF_ACCELERATED:0) | NX_WF_AFFECTED_BY_HANDBRAKE | flags;
	wheelDesc[3].wheelFlags |= ((vinfo.driven==BackDriven)?NX_WF_ACCELERATED:0) | NX_WF_AFFECTED_BY_HANDBRAKE | flags;

	vehicleDesc.steeringSteerPoint.set(1.8, 0, 0);
	vehicleDesc.steeringTurnPoint.set(-1.5, 0, 0);

	NxVehicle* vehicle = NxVehicle::createVehicle(nxScene, &vehicleDesc, name);
	NxQuat q;
	// 少转过90度,可能会有问题
	q.fromAngleAxis(90.0f, NxVec3(0.0f, 1.0f, 0.0f));
	vehicle->getActor()->setGlobalOrientationQuat(q);

	vehicle->mVInfo = vinfo;
	vehicle->setOilAmount(vinfo.oilAmount);

	// 加到队列中
	mAllVehicle.pushBack(vehicle);
	mIsLive.pushBack(0);
//	miUserVehicle = mAllVehicle.size() - 1;

	return vehicle;
}

int NxAllVehicle::setCurrentUser(const std::string& name)	// 设置当前用户
{
	int n = mAllVehicle.size();
	for (int i=0; i<n; i++)
	{
		if (mAllVehicle[i]->getVehicleName() == name)
		{
			miUserVehicle = i;
			return 1;
		}
	}

	return 0;
}

// 得到当前用可操作的车辆的指针
NxVehicle* NxAllVehicle::getUserVehicle()
{
	if (miUserVehicle == -1)
	{
		return NULL;
	}
	
	return mAllVehicle[miUserVehicle];
}

void NxAllVehicle::setOilAmount(int oilAmount)
{
	mAllVehicle[miUserVehicle]->setOilAmount(oilAmount);
}

int NxAllVehicle::getOilAmount()
{
	return mAllVehicle[miUserVehicle]->getOilAmount();
}

void NxAllVehicle::setLoopNum(int nLoop)
{
	mAllVehicle[miUserVehicle]->setLoopNum(nLoop);
}

int NxAllVehicle::getLoopNum()
{
	return mAllVehicle[miUserVehicle]->getLoopNum();
}

// 当前汽车已输
void NxAllVehicle::killUserVehicle()
{
	if (miUserVehicle == -1)
	{
		return;
	}
	mIsLive[miUserVehicle] = ++mCountDead;
}

// 转换到下一辆车为用户控制的车.此函数只用来调试用,在游戏中不会使用
NxVehicle* NxAllVehicle::changeUserVehicle()
{
	if (mAllVehicle.size() == 0)
	{
		return NULL;
	}
	if ((unsigned)miUserVehicle >= mAllVehicle.size()-1)
	{
		miUserVehicle = 0;
	}
	else
	{
		miUserVehicle++;
	}

	return mAllVehicle[miUserVehicle];
}


void NxAllVehicle::control(int steering, int acceleration, bool handBrake)
{
	if (miUserVehicle == -1)
	{
		return;
	}
	mAllVehicle[miUserVehicle]->control(steering, false, acceleration, true, handBrake);
}

void NxAllVehicle::standUp()
{
	if (miUserVehicle == -1)
	{
		return;
	}
	mAllVehicle[miUserVehicle]->standUp();
}

void NxAllVehicle::setToBeginning(const NxVec3& pos)
{
	if (miUserVehicle == -1)
	{
		return;
	}
	mAllVehicle[miUserVehicle]->setToBeginning(pos); 
}

void NxAllVehicle::updateAllVehicle(NxReal lastTimeStepSize)
{
	int n = mAllVehicle.size();
	for (int i=0; i<n; i++)
	{
		mAllVehicle[i]->updateVehicle(lastTimeStepSize);
	}
}

AllVehicleInfoNx* NxAllVehicle::getVehicleInfo()
{
	if (mSizeAllInfoNx < mAllVehicle.size())
	{
		mSizeAllInfoNx = mAllVehicle.size();
		delete[] mAllInfoNx;
		mAllInfoNx = new AllVehicleInfoNx[mSizeAllInfoNx];
	}
	for (int i=0; i<mSizeAllInfoNx; i++)
	{
		mAllInfoNx[i] = mAllVehicle[i]->getInfoForOgre();
	}
	return mAllInfoNx;
}

float NxAllVehicle::getUserVelocity()
{
	if (miUserVehicle >= 0)
	{
		return mAllVehicle[miUserVehicle]->getDriveVelocity();
	}
}

void NxAllVehicle::setToSlip()	//设为打滑效果
{
	this->mAllVehicle[miUserVehicle]->setToSlip();
}

void NxAllVehicle::cancleSlip()	// 无打滑效果
{
	this->mAllVehicle[miUserVehicle]->cancleSlip();
}

void NxAllVehicle::setMaxVelocity(float velocity)
{
	this->mAllVehicle[miUserVehicle]->setMaxVelocity(velocity);
}
