#ifndef NX_VEHICLE
#define NX_VEHICLE

#include "NxVehicleDesc.h"
#include "NxWheel.h"
#include "NxVehicleMotor.h"
#include "NxVehicleGears.h"

#include <NxScene.h>
#include <NxArray.h>
#include <NxUserContactReport.h>
#include <string>

#define NUM_TRAIL_POINTS 1600

enum Driven {FrontDriven, BackDriven};
enum VehicleType {Car, Jeep, Toy};

class VehicleInfo
{
public:
	NxReal width;	// 宽
	NxReal length;	// 长
	NxReal height;	// 高
	NxReal mass;	// 重量
	NxReal maxAcceleraion;	// 最大加速度
	NxReal maxVelocity;	// 最大速度
	NxReal maxHandBraking;	// 最大手刹
	NxReal steerablity;	// 转向性能
	NxReal maxSteeringAngle;	// 最大转向角
	Driven driven;	// 前驱/后驱
	VehicleType type;	// 车类型
	int oilAmount;	// 油量
	VehicleInfo() : width(2.3), length(5), height(1.2), 
		mass(100), maxAcceleraion(2), maxVelocity(50), maxHandBraking(1000), 
		steerablity(1), driven(BackDriven), type(Car), oilAmount(200) 
	{
	}
};

struct AllVehicleInfoNx
{
	NxVec3 vepos;	// 车位置
	NxQuat verot;	// 车角度
	NxVec3 campos;	// 相机位置
	NxQuat camrot;	// 相机角度
	float angle0;	// 左前轮角度
	float angle1;	// 右前轮角度
	float angle2;	// 左后轮角度
	float angle3;	// 右后轮角度
	std::string name;
};

class NxVehicle 
{
public:
	// 车子过起跑线共有7种状态(进入的方向不同也不一样),在起跑线外(1),在一个半起跑线内(2/7, 4/5),在两个半起跑线上(3/6)
	int mStatusScratch;	// 车在起跑线上的状态,1, 2, 3, 4, 5, 6, 7
	void* userData;
	VehicleInfo mVInfo;
private:
	float mAngle[4];	// 四个轮子的转向角
	std::string mName;	// 用户名
	int mCurOilAmount;	// 当前油量
	int mCurLoop;	// 当前圈数
	NxArray<NxWheel*> _wheels;

	NxActor* _bodyActor;
	NxScene* _nxScene;

	NxVehicleMotor* _vehicleMotor;
	NxVehicleGears* _vehicleGears;

	NxReal _steeringWheelState;
	NxReal _accelerationPedal;	// 相当于力臂
	NxReal _brakePedal;

	NxReal _digitalSteeringDelta;
	NxVec3 _steeringTurnPoint;
	NxVec3 _steeringSteerPoint;
	NxReal _steeringMaxAngleRad;
	NxReal _motorForce;	// 动力
	NxReal _transmissionEfficiency;	// 传送率,因为力什么的可能会有损失
	NxReal _differentialRatio;

	NxVec3 _localVelocity;
	NxMaterial* _carMaterial;

	NxVec3 _trailBuffer[NUM_TRAIL_POINTS];
	NxU32 _nextTrailSlot;
	NxReal _lastTrailTime;

	NxActor* _mostTouchedActor;
private:
	NxVehicle();
	// 找到与四个轮子接触最多的NxActor,赋值给_mostTouchedActor
	void _computeMostTouchedActor();
	// 保存在_localVelocity中, 一堆位置角度变换,
	void _computeLocalVelocity();
	// 先做_updateRpms()完全一样的事,现计算出torque,有_vehicleMotor时还有点不太明白
	NxReal _computeAxisTorque();
	// 有点怪,取所有带有标记NX_WF_ACCELERATED的轮子线性速度的平均值
	// 从这个数可以看出,至少一个轮子带有这种标志,否则会有0/0
	NxReal _computeRpmFromWheels();
	//根据传入的rpm(比如_computeRpmFromWheels()的返回值)计算并返回真正的rpm, 一般为*GearRatio*differentialRatio之后
	NxReal _computeMotorRpm(NxReal rpm);
	//先计算_computeRpmFromWheels(),再传入_computeMotorRpm(NxReal rpm),计算出rpm之后,再设置_vehicleMotor中的_rpm为这个值
	void _updateRpms();

	// 取得相应Gear下的Ratio
	NxReal _getGearRatio();

	// 可能用于转向表什么的,改变的是_steeringWheelState,改变的大小与_digitalSteeringDelta有关
	void _controlSteering(NxReal steering, bool analogSteering);
	// 改变_releaseBraking, _accelerationPedal, _brakePedalChanged, _brakePedal
	void _controlAcceleration(NxReal acceleration, bool analogAcceleration);
	// 创建一辆车为一个NxActor, 添加了四个轮子,最后调用control,进行一些初始化
	static NxVehicle*		_createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc);
public:
	~NxVehicle();
	// 如果用物理引擎提供的NxWheelShape, 这个函数没什么用
	void handleContactPair(NxContactPair& pair, NxU32 carIndex);
	//更新属性
	void updateVehicle(NxReal lastTimeStepSize);
	// 调用一堆_control之类的,给许多变量赋值
	void control (NxReal steering, bool analogSteering, NxReal acceleration, bool analogAcceleration, bool handBrake);
	// 只是使_vehicleGears中_curGear值加1,如果成功
	void gearUp();
	// 只是使_vehicleGears中_curGear值减1,如果成功
	void gearDown();

	std::string getVehicleName();	// 得到该车用户名字

	void setToSlip();	//设为打滑效果
	void cancleSlip();	// 无打滑效果
	void setMaxVelocity(float velocity)
	{
		mVInfo.maxVelocity = velocity;
	}

	// 随机加力,这里并没什么用
	void applyRandomForce();
	// 车复位,一堆位置角度变换
	void standUp();
	void setToBeginning(const NxVec3& pos);
	// 设置刹车大小
	void setBrake(NxReal brake);
	void setOilAmount(int oilAmount)
	{
		mCurOilAmount = oilAmount;
	}
	int getOilAmount()
	{
		return mCurOilAmount;
	}
	void setLoopNum(int nLoop)
	{
		mCurLoop = nLoop;
	}
	int getLoopNum()
	{
		return mCurLoop;
	}

	AllVehicleInfoNx getInfoForOgre();	// 得到OGRE绘制所需的车的物理信息
	NxReal getDriveVelocity()
	{
		if (_localVelocity.x > 0)
		{
			return _localVelocity.x; 
		}
		else
		{
			return 0;
		}
	}


	const NxVehicleMotor* getMotor() const { return _vehicleMotor; }
	const NxVehicleGears* getGears() const { return _vehicleGears; }
	NxActor* getActor() { return _bodyActor; }

	NxU32 getNbWheels() { return _wheels.size(); }
	const NxWheel* getWheel(NxU32 i) { NX_ASSERT(i < _wheels.size()); return _wheels[i]; }

	NxMat34 getGlobalPose() { return _bodyActor->getGlobalPose(); }

	// 基本上_createVehicle, 另外还创建了它的children,如果有
	static NxVehicle* createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc, std::string name);
};

#endif