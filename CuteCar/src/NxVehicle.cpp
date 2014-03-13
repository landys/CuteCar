#include <cstdio>

#include <map>

#include "NxVehicle.h"
#include "NxWheel.h"

#include <NxActorDesc.h>
#include <NxAllocateable.h>
#include <NxBoxShape.h>
#include <NxConvexShape.h>
#include <NxConvexShapeDesc.h>
#include <NxConvexMesh.h>

#define TRAIL_FREQUENCY 0.025f

NxVehicle::NxVehicle() : _steeringWheelState(0), _nxScene(NULL), _carMaterial(NULL)
{ 
	memset(_trailBuffer, 0, sizeof(NxVec3) * NUM_TRAIL_POINTS);
	_nextTrailSlot = 0;
	_lastTrailTime = 0.0f;
	for (int i=0; i<4; i++)
	{
		mAngle[i] = 0.0f;
	}
	mCurLoop = 0;
	mStatusScratch = 1;	// 初始化位于第一种状态
}

NxVehicle::~NxVehicle()
{
	if (_carMaterial)
		_nxScene->releaseMaterial(*_carMaterial);

	if (_bodyActor)
		_nxScene->releaseActor(*_bodyActor);

	for(NxU32 i = 0; i < _wheels.size(); i++)
	{
		if(_wheels[i])
		{
			delete _wheels[i];
			_wheels[i] = NULL;
		}
	}
}

NxVehicle* NxVehicle::_createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc)//, Ogre::SceneNode* parNode)
{
	if(vehicleDesc == NULL)
		return NULL;


	if(!vehicleDesc->isValid())
	{
//		printf("Vehicle Desc not valid!!\n");
		return NULL;
	}
	
	NxVehicle* vehicle = new NxVehicle();
	vehicle->userData = vehicleDesc->userData;

	vehicle->_nxScene = scene;

	if(vehicle->_carMaterial == NULL)
	{
		NxMaterialDesc carMaterialDesc;
		carMaterialDesc.dynamicFriction = 0;
		carMaterialDesc.staticFriction = 0;
		carMaterialDesc.restitution = 0;
		carMaterialDesc.frictionCombineMode = NX_CM_MULTIPLY;
		vehicle->_carMaterial = scene->createMaterial(carMaterialDesc);
	}
	
	NxActorDesc actorDesc;
	for(NxU32 i = 0; i < vehicleDesc->carShapes.size(); i++)
	{
		actorDesc.shapes.pushBack(vehicleDesc->carShapes[i]);
		if (actorDesc.shapes[i]->materialIndex == 0)
			actorDesc.shapes[i]->materialIndex = vehicle->_carMaterial->getMaterialIndex();
	}

	NxBodyDesc bodyDesc;
	bodyDesc.mass = vehicleDesc->mass;
	actorDesc.body = &bodyDesc;
	actorDesc.globalPose.t = vehicleDesc->position;

	vehicle->_bodyActor = scene->createActor(actorDesc);
	if(vehicle->_bodyActor == NULL)
	{
		delete vehicle;
		return NULL;
	}
	vehicle->_bodyActor->userData = vehicle;

	if(vehicleDesc->motorDesc != NULL)
	{
		vehicle->_vehicleMotor = NxVehicleMotor::createMotor(*vehicleDesc->motorDesc);
		if(vehicle->_vehicleMotor == NULL)
		{
			delete vehicle;
			return NULL;
		}
		vehicle->_motorForce = 0;
	}
	else 
	{
		vehicle->_vehicleMotor = NULL;
		vehicle->_motorForce = vehicleDesc->motorForce;
	}

	if(vehicleDesc->gearDesc != NULL)
	{
		vehicle->_vehicleGears = NxVehicleGears::createVehicleGears(*vehicleDesc->gearDesc);
		if (vehicle->_vehicleGears == NULL) 
		{
//			printf("Vehicle gear-creation failed\n");
			delete vehicle;
			return NULL;
		}
	}
	else 
	{
		vehicle->_vehicleGears = NULL;
	}

	for(NxU32 i = 0; i < vehicleDesc->carWheels.size(); i++)
	{
		NxWheel* wheel = NxWheel::createWheel(vehicle->_bodyActor, vehicleDesc->carWheels[i]);//vehicle->m_pNode);
		if(wheel)
		{
			vehicle->_wheels.pushBack(wheel);
		}
		else 
		{
			delete vehicle;
			return NULL;
		}
	}

	vehicle->_digitalSteeringDelta		= vehicleDesc->digitalSteeringDelta;
	vehicle->_steeringSteerPoint		= vehicleDesc->steeringSteerPoint;
	vehicle->_steeringTurnPoint			= vehicleDesc->steeringTurnPoint;
	vehicle->_steeringMaxAngleRad		= NxMath::degToRad(vehicleDesc->steeringMaxAngle);
	vehicle->_transmissionEfficiency	= vehicleDesc->transmissionEfficiency;
	vehicle->_differentialRatio			= vehicleDesc->differentialRatio;
	vehicle->_bodyActor->setCMassOffsetLocalPosition(vehicleDesc->centerOfMass);

	//don't go to sleep.
	vehicle->_bodyActor->wakeUp(1e10);

	vehicle->control(0, true, 0, true, false);

	return vehicle;
}

NxVehicle* NxVehicle::createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc, std::string name)
{
	if (vehicleDesc == NULL)
	{
		return NULL;
	}
	
	NxVehicle* vehicle = NxVehicle::_createVehicle(scene, vehicleDesc);
	vehicle->mName = name;
	return vehicle;
}


AllVehicleInfoNx NxVehicle::getInfoForOgre()
{
	static bool isFirst = true;
	static int curIndex = 0;
	static NxQuat rotBuffer[30];
	
	AllVehicleInfoNx allv;

	NxVec3 nxPos;
	NxQuat nxRot;

	nxPos = _bodyActor->getGlobalPosition();
	nxRot = _bodyActor->getGlobalOrientationQuat();

	
	nxRot.multiply(nxRot, NxQuat(90.0f, NxVec3(0, 1, 0)));

	allv.verot = nxRot;
	allv.vepos = nxPos;

	NxQuat rot = _bodyActor->getGlobalOrientationQuat();

	NxVec3 front(1,0,0);
	rot.rotate(front);

	front.normalize();

	NxReal dotproduct  = front.x;

	NxReal angle = NxMath::sign(-front.z) * NxMath::acos(dotproduct);

	rot.fromAngleAxis(NxMath::radToDeg(angle), NxVec3(0,1,0));

	NxVec3 revec;
	
	revec = NxVec3(-NxMath::cos(angle)*30, 13, NxMath::sin(angle)*30);

	
	int i = 0;
	if (isFirst)
	{
		for (i=0; i<30; i++)
		{
			rotBuffer[i] = rot;
		}
	}
	
	allv.camrot = rotBuffer[curIndex];
	rotBuffer[curIndex++] = rot;
	if (curIndex >= 30)
	{
		curIndex = 0;
	}
	
	//allv.camrot = rot;
	allv.campos = revec;

	allv.angle0 = mAngle[0];
	allv.angle1 = mAngle[1];
	allv.angle2 = mAngle[2];
	allv.angle3 = mAngle[3];
	allv.name = this->mName;

	return allv;
}

void NxVehicle::handleContactPair(NxContactPair& pair, NxU32 carIndex)
{
}

void NxVehicle::updateVehicle(NxReal lastTimeStepSize)
{
	NxReal distanceSteeringAxisCarTurnAxis = _steeringSteerPoint.x  - _steeringTurnPoint.x;
	NX_ASSERT(_steeringSteerPoint.z == _steeringTurnPoint.z);
	NxReal distance2 = 0;
	if (NxMath::abs(_steeringWheelState) > 0.01f)
		distance2 = distanceSteeringAxisCarTurnAxis / NxMath::tan(_steeringWheelState * _steeringMaxAngleRad);

// 车痕
	_lastTrailTime += lastTimeStepSize;

	if(_lastTrailTime > TRAIL_FREQUENCY)
	{
		_lastTrailTime = 0.0f;
	}

	NxU32 nbTouching = 0;
	NxU32 nbNotTouching = 0;
	NxU32 nbHandBrake = 0;
	
	NxReal angle[4];
	for(NxU32 i = 0; i < _wheels.size(); i++)
	{
		NxWheel* wheel = _wheels[i];

		if (_lastTrailTime  == 0.0f)
		{
			if(_wheels[i]->hasGroundContact())
			{
				if (++_nextTrailSlot >= NUM_TRAIL_POINTS)
					_nextTrailSlot = 0;
				_trailBuffer[_nextTrailSlot] = _bodyActor->getGlobalPose() * _wheels[i]->getGroundContactPos();
			}
		}

		if(wheel->getWheelFlag(NX_WF_STEERABLE_INPUT))
		{
			if(distance2 != 0)
			{
				NxReal xPos = wheel->getWheelPos().x;
				NxReal zPos = wheel->getWheelPos().z;
				NxReal dz = -zPos + distance2;
				NxReal dx = xPos - _steeringTurnPoint.x;
				angle[i] = NxMath::atan(dx/dz);
				wheel->setAngle(angle[i]);
			} 
			else 
			{
				angle[i] = 0.f;
				wheel->setAngle(angle[i]);
			}
		} 
		else if(wheel->getWheelFlag(NX_WF_STEERABLE_AUTO))
		{
			NxVec3 localVelocity = _bodyActor->getLocalPointVelocity(wheel->getWheelPos());
			NxQuat local2Global = _bodyActor->getGlobalOrientationQuat();
			local2Global.inverseRotate(localVelocity);

			localVelocity.y = 0;
			if(localVelocity.magnitudeSquared() < 0.01f)
			{
				angle[i] = 0.f;
				wheel->setAngle(angle[i]);
			} 
			else
			{
				localVelocity.normalize();

				if(localVelocity.x < 0)
					localVelocity = -localVelocity;
				angle[i] = NxMath::clamp(atan(localVelocity.z / localVelocity.x), 0.3f, -0.3f);
				wheel->setAngle(angle[i]);
			}
		}

		// now the acceleration part
		if(!wheel->getWheelFlag(NX_WF_ACCELERATED))
			continue;

		if (!wheel->hasGroundContact())
		{
			nbNotTouching++;
		} else {
			nbTouching++;
		}
	}

	if(_wheels[0]->getWheelFlag(NX_WF_STEERABLE_INPUT))
	{
		mAngle[0] = angle[0];
		mAngle[2] = angle[1];
		mAngle[1] = 0.0f;
		mAngle[3] = 0.0f;
	}
	else
	{
		mAngle[0] = 0.0f;
		mAngle[2] = 0.0f;
		mAngle[1] = angle[2];
		mAngle[3] = angle[3];
	}
	
	NxReal motorTorque = 0.0f; 
	if(nbTouching && NxMath::abs(_accelerationPedal) > 0.01f) 
	{
		NxReal axisTorque = _computeAxisTorque();
		NxReal wheelTorque = axisTorque / (NxReal)(_wheels.size());// - nbHandBrake);

		NxReal wheelTorqueNotTouching = nbNotTouching>0?wheelTorque*(NxMath::pow(0.5f, (NxReal)nbNotTouching)):0;
		NxReal wheelTorqueTouching = wheelTorque - wheelTorqueNotTouching;
		motorTorque = wheelTorqueTouching / (NxReal)nbTouching; 
	} else {
		_updateRpms();
	}

	for(NxU32 i = 0; i < _wheels.size(); i++) 
	{
		NxWheel* wheel = _wheels[i];
		wheel->tick(motorTorque*5, _brakePedal);
	}
}

//finds the actor that is touched by most wheels.
void NxVehicle::_computeMostTouchedActor()
{
	std::map<NxActor*, NxU32> actors;
	typedef std::map<NxActor*, NxU32> Map;
	for(NxU32 i = 0; i < _wheels.size(); i++)
	{
		NxActor* curActor = _wheels[i]->getTouchedActor();
		Map::iterator it = actors.find(curActor);
		if (it == actors.end())
		{
			actors[curActor] = 1;
		} else {
			it->second++;
		}
	}

	NxU32 count = 0;
	_mostTouchedActor = NULL;
	for(Map::iterator it = actors.begin(); it != actors.end(); ++it)
	{
		if(it->second > count)
		{
			count = it->second;
			_mostTouchedActor = it->first;
		}
	}
}

void NxVehicle::_controlSteering(NxReal steering, bool analogSteering)
{
	if(analogSteering)
	{
		_steeringWheelState = steering;
	} else if (NxMath::abs(steering) > 0.0001f) {
		_steeringWheelState += NxMath::sign(steering) * _digitalSteeringDelta;
	} else if (NxMath::abs(_steeringWheelState) > 0.0001f) {
		_steeringWheelState -= NxMath::sign(_steeringWheelState) * _digitalSteeringDelta;
	}
	_steeringWheelState = NxMath::clamp(_steeringWheelState, 1.f, -1.f);
}

void NxVehicle::_computeLocalVelocity()
{
	_computeMostTouchedActor();
	NxVec3 relativeVelocity;
	if (_mostTouchedActor == NULL || !_mostTouchedActor->isDynamic())
	{
		relativeVelocity = _bodyActor->getLinearVelocity();
	} 
	else 
	{
		relativeVelocity = _bodyActor->getLinearVelocity() - _mostTouchedActor->getLinearVelocity();
	}

	//???????????????????????????????????????????????
	NxQuat rotation = _bodyActor->getGlobalOrientationQuat();
	NxQuat global2Local;
	_localVelocity = relativeVelocity;
	rotation.inverseRotate(_localVelocity);
}

void NxVehicle::_controlAcceleration(NxReal acceleration, bool analogAcceleration)
{
	_accelerationPedal = NxMath::clamp(acceleration, 1.f, -1.f);
}

void NxVehicle::setBrake(NxReal brake)
{
}

void NxVehicle::control(NxReal steering, bool analogSteering, NxReal acceleration, bool analogAcceleration, bool handBrake)
{
	_controlSteering(steering, analogSteering);
	_computeLocalVelocity();

	if (handBrake)
	{
		_brakePedal = this->mVInfo.maxHandBraking;
	}
	else
	{
		_brakePedal = 160;
	}
	if (this->_bodyActor->getGlobalPosition().y > 10)
	{
		acceleration = 0;
		//getActor()->setLinearVelocity(NxVec3(0,0,0));
		getActor()->setAngularVelocity(NxVec3(0,0,0));
	}
	else if (this->_bodyActor->getGlobalPosition().y > 4)
	{
		acceleration = 0;
	}
	else
	{
		float x = _localVelocity.x;
		float y = mVInfo.maxVelocity * 3 / 5;
		if (x > 0)
		{
			if (mVInfo.maxVelocity <= x)
			{
				acceleration = 0;
			}
			else if (x < y)
			{
				acceleration *= (0.5 +  (y - x) / y);
			}
		}
		else
		{
			acceleration *= 0.24;
		}
	}
	
	_controlAcceleration(acceleration, analogAcceleration);
}

NxReal NxVehicle::_computeAxisTorque()
{
	if(_vehicleMotor != NULL)
	{
		NxReal rpm = _computeRpmFromWheels();
		NxReal motorRpm = _computeMotorRpm(rpm);
		_vehicleMotor->setRpm(motorRpm);
		//根据_rpm,_vehicleMotor会通过一些插值运算等算出torque
		NxReal torque = _accelerationPedal * _vehicleMotor->getTorque();
		NxReal v = _bodyActor->getLinearVelocity().magnitude();

		return torque * _getGearRatio() * _differentialRatio * _transmissionEfficiency;
	}
	else 
	{
		_computeRpmFromWheels();
		return _accelerationPedal * _motorForce;
	}
}

NxReal NxVehicle::_computeRpmFromWheels()
{
	NxReal wheelRpms = 0;
	NxI32 nbWheels = 0;
	for(NxU32 i = 0; i < _wheels.size(); i++)
	{
		NxWheel* wheel = _wheels[i];
		if (wheel->getWheelFlag(NX_WF_ACCELERATED))
		{
			nbWheels++;
			wheelRpms += wheel->getRpm();
		}
	}
	return wheelRpms / (NxReal)nbWheels;
}

NxReal NxVehicle::_computeMotorRpm(NxReal rpm)
{
	NxReal temp = _getGearRatio() * _differentialRatio;
	NxReal motorRpm = rpm * temp;
	if(_vehicleMotor)
	{
		NxI32 change;
		if(_vehicleGears && (change = _vehicleMotor->changeGears(_vehicleGears, 0.2f)))
		{
			if(change == 1)
			{
				gearUp();
			} else {
				NX_ASSERT(change == -1);
				gearDown();
			}
		}
		temp = _getGearRatio() * _differentialRatio;
		motorRpm = NxMath::max(rpm * temp, _vehicleMotor->getMinRpm());
	}
	return motorRpm;
}

NxReal NxVehicle::_getGearRatio()
{
	if(_vehicleGears == NULL)
	{
		return 1;
	} 
	else 
	{
		return _vehicleGears->getCurrentRatio();
	}
}

void NxVehicle::gearUp()
{
	if (_vehicleGears)
	{
//		printf("Changing gear from %d to", _vehicleGears->getGear());
		_vehicleGears->gearUp();
//		printf(" %d\n", _vehicleGears->getGear());
	} 
	else 
	{
//		printf("gearUp not supported if no gears available\n");
	}
}
void NxVehicle::gearDown()
{
	if(_vehicleGears)
	{
//		printf("Changing gear from %d to", _vehicleGears->getGear());
		_vehicleGears->gearDown();
//		printf(" %d\n", _vehicleGears->getGear());
	} 
	else 
	{
//		printf("gearDown not supported if no gears available\n");
	}
}

void NxVehicle::applyRandomForce()
{
	NxVec3 pos(NxMath::rand(-4.f,4.f),NxMath::rand(-4.f,4.f),NxMath::rand(-4.f,4.f));
	NxReal force = NxMath::rand(_bodyActor->getMass()*0.5f, _bodyActor->getMass() * 2.f);
	_bodyActor->addForceAtLocalPos(NxVec3(0, force*100.f, 0), pos);
}

void NxVehicle::standUp()
{
	NxVec3 pos = getActor()->getGlobalPosition() + NxVec3(0,2,0);
//	printf("pos = (%f, %f, %f)", pos.x, pos.y, pos.z);
	NxQuat rot = getActor()->getGlobalOrientationQuat();
//	printf("rot = (%f, %f, %f, %f)", rot.x, rot.y, rot.z, rot.w);
	NxVec3 front(1,0,0);
	rot.rotate(front);
//	printf("rot = (%f, %f, %f, %f)", rot.x, rot.y, rot.z, rot.w);
//	printf("front = (%f, %f, %f)", front.x, front.y, front.z);
	front.y = 0;
	front.normalize();
//	printf("front = (%f, %f, %f)", front.x, front.y, front.z);
	NxReal dotproduct  = front.x;

	NxReal angle = NxMath::sign(-front.z) * NxMath::acos(dotproduct);
//	printf("angle = %f\n", angle);
	rot.fromAngleAxis(NxMath::radToDeg(angle), NxVec3(0,1,0));
//	printf("rot = (%f, %f, %f, %f)\n", rot.x, rot.y, rot.z, rot.w);
	getActor()->setGlobalPosition(pos);
	getActor()->setGlobalOrientationQuat(rot);
	getActor()->setLinearVelocity(NxVec3(0,0,0));
	getActor()->setAngularVelocity(NxVec3(0,0,0));
}

void NxVehicle::setToBeginning(const NxVec3& pos)
{
	NxQuat rot = NxQuat(90, NxVec3(0, 1, 0));
	NxVec3 front(1,0,0);
	rot.rotate(front);

	front.y = 0;
	front.normalize();
	NxReal dotproduct  = front.x;

	NxReal angle = NxMath::sign(-front.z) * NxMath::acos(dotproduct);
	rot.fromAngleAxis(NxMath::radToDeg(angle), NxVec3(0,1,0));
	getActor()->setGlobalPosition(pos);
	getActor()->setGlobalOrientationQuat(rot);
	getActor()->setLinearVelocity(NxVec3(0,0,0));
	getActor()->setAngularVelocity(NxVec3(0,0,0));

	this->mCurOilAmount = mVInfo.oilAmount;
	this->mStatusScratch = 1;
	this->mVInfo.maxVelocity = 65.f;
	this->mVInfo.oilAmount = 200;
	this->mCurLoop = 0;
}

void NxVehicle::_updateRpms()
{
	NxReal rpm = _computeRpmFromWheels();
	if(_vehicleMotor != NULL)
	{
		NxReal motorRpm = _computeMotorRpm(rpm);
		_vehicleMotor->setRpm(motorRpm);
	}
}

std::string NxVehicle::getVehicleName()	// 得到该车用户名字
{
	return mName;
}

void NxVehicle::setToSlip()	//设为打滑效果
{
	NxTireFunctionDesc tireF;
	tireF.asymptoteValue = 0;
	tireF.extremumValue = 0.01;
	tireF.extremumSlip = 50.f;
	tireF.asymptoteSlip = 100.f;

	_wheels[2]->setSideSlipEffect(tireF);
	_wheels[3]->setSideSlipEffect(tireF);
}

void NxVehicle::cancleSlip()	// 无打滑效果
{
	NxTireFunctionDesc tireF;
	tireF.asymptoteValue = 0.01;
	tireF.extremumValue = 0.02;
	tireF.extremumSlip = 1.0;
	tireF.asymptoteSlip = 2.0;

	_wheels[2]->setSideSlipEffect(tireF);
	_wheels[3]->setSideSlipEffect(tireF);
}