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
	NxReal width;	// ��
	NxReal length;	// ��
	NxReal height;	// ��
	NxReal mass;	// ����
	NxReal maxAcceleraion;	// �����ٶ�
	NxReal maxVelocity;	// ����ٶ�
	NxReal maxHandBraking;	// �����ɲ
	NxReal steerablity;	// ת������
	NxReal maxSteeringAngle;	// ���ת���
	Driven driven;	// ǰ��/����
	VehicleType type;	// ������
	int oilAmount;	// ����
	VehicleInfo() : width(2.3), length(5), height(1.2), 
		mass(100), maxAcceleraion(2), maxVelocity(50), maxHandBraking(1000), 
		steerablity(1), driven(BackDriven), type(Car), oilAmount(200) 
	{
	}
};

struct AllVehicleInfoNx
{
	NxVec3 vepos;	// ��λ��
	NxQuat verot;	// ���Ƕ�
	NxVec3 campos;	// ���λ��
	NxQuat camrot;	// ����Ƕ�
	float angle0;	// ��ǰ�ֽǶ�
	float angle1;	// ��ǰ�ֽǶ�
	float angle2;	// ����ֽǶ�
	float angle3;	// �Һ��ֽǶ�
	std::string name;
};

class NxVehicle 
{
public:
	// ���ӹ������߹���7��״̬(����ķ���ͬҲ��һ��),����������(1),��һ������������(2/7, 4/5),����������������(3/6)
	int mStatusScratch;	// �����������ϵ�״̬,1, 2, 3, 4, 5, 6, 7
	void* userData;
	VehicleInfo mVInfo;
private:
	float mAngle[4];	// �ĸ����ӵ�ת���
	std::string mName;	// �û���
	int mCurOilAmount;	// ��ǰ����
	int mCurLoop;	// ��ǰȦ��
	NxArray<NxWheel*> _wheels;

	NxActor* _bodyActor;
	NxScene* _nxScene;

	NxVehicleMotor* _vehicleMotor;
	NxVehicleGears* _vehicleGears;

	NxReal _steeringWheelState;
	NxReal _accelerationPedal;	// �൱������
	NxReal _brakePedal;

	NxReal _digitalSteeringDelta;
	NxVec3 _steeringTurnPoint;
	NxVec3 _steeringSteerPoint;
	NxReal _steeringMaxAngleRad;
	NxReal _motorForce;	// ����
	NxReal _transmissionEfficiency;	// ������,��Ϊ��ʲô�Ŀ��ܻ�����ʧ
	NxReal _differentialRatio;

	NxVec3 _localVelocity;
	NxMaterial* _carMaterial;

	NxVec3 _trailBuffer[NUM_TRAIL_POINTS];
	NxU32 _nextTrailSlot;
	NxReal _lastTrailTime;

	NxActor* _mostTouchedActor;
private:
	NxVehicle();
	// �ҵ����ĸ����ӽӴ�����NxActor,��ֵ��_mostTouchedActor
	void _computeMostTouchedActor();
	// ������_localVelocity��, һ��λ�ýǶȱ任,
	void _computeLocalVelocity();
	// ����_updateRpms()��ȫһ������,�ּ����torque,��_vehicleMotorʱ���е㲻̫����
	NxReal _computeAxisTorque();
	// �е��,ȡ���д��б��NX_WF_ACCELERATED�����������ٶȵ�ƽ��ֵ
	// ����������Կ���,����һ�����Ӵ������ֱ�־,�������0/0
	NxReal _computeRpmFromWheels();
	//���ݴ����rpm(����_computeRpmFromWheels()�ķ���ֵ)���㲢����������rpm, һ��Ϊ*GearRatio*differentialRatio֮��
	NxReal _computeMotorRpm(NxReal rpm);
	//�ȼ���_computeRpmFromWheels(),�ٴ���_computeMotorRpm(NxReal rpm),�����rpm֮��,������_vehicleMotor�е�_rpmΪ���ֵ
	void _updateRpms();

	// ȡ����ӦGear�µ�Ratio
	NxReal _getGearRatio();

	// ��������ת���ʲô��,�ı����_steeringWheelState,�ı�Ĵ�С��_digitalSteeringDelta�й�
	void _controlSteering(NxReal steering, bool analogSteering);
	// �ı�_releaseBraking, _accelerationPedal, _brakePedalChanged, _brakePedal
	void _controlAcceleration(NxReal acceleration, bool analogAcceleration);
	// ����һ����Ϊһ��NxActor, ������ĸ�����,������control,����һЩ��ʼ��
	static NxVehicle*		_createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc);
public:
	~NxVehicle();
	// ��������������ṩ��NxWheelShape, �������ûʲô��
	void handleContactPair(NxContactPair& pair, NxU32 carIndex);
	//��������
	void updateVehicle(NxReal lastTimeStepSize);
	// ����һ��_control֮���,����������ֵ
	void control (NxReal steering, bool analogSteering, NxReal acceleration, bool analogAcceleration, bool handBrake);
	// ֻ��ʹ_vehicleGears��_curGearֵ��1,����ɹ�
	void gearUp();
	// ֻ��ʹ_vehicleGears��_curGearֵ��1,����ɹ�
	void gearDown();

	std::string getVehicleName();	// �õ��ó��û�����

	void setToSlip();	//��Ϊ��Ч��
	void cancleSlip();	// �޴�Ч��
	void setMaxVelocity(float velocity)
	{
		mVInfo.maxVelocity = velocity;
	}

	// �������,���ﲢûʲô��
	void applyRandomForce();
	// ����λ,һ��λ�ýǶȱ任
	void standUp();
	void setToBeginning(const NxVec3& pos);
	// ����ɲ����С
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

	AllVehicleInfoNx getInfoForOgre();	// �õ�OGRE��������ĳ���������Ϣ
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

	// ������_createVehicle, ���⻹����������children,�����
	static NxVehicle* createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc, std::string name);
};

#endif