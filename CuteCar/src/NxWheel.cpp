#include "NxWheel.h"

#include <cstdio>

#include <NxCooking.h>
#include "Stream.h"


NxWheel* NxWheel::createWheel(NxActor* actor, NxWheelDesc* wheelDesc)//, Ogre::SceneNode* node) 
{
	return new NxWheel(actor, wheelDesc);
}

NxWheel::NxWheel(NxActor* a, NxWheelDesc* wheelDesc) : actor(a)//, m_pNode(NULL), m_pParNode(node)
{
	NxScene* scene = &actor->getScene();
	//create a shared car wheel material to be used by all wheels
	static NxMaterial* wsm = 0;
	if(!wsm)
	{
		NxMaterialDesc m;
		//m.flags |= NX_MF_DISABLE_FRICTION;	// ²»½ûÄ¦²Á
		m.dynamicFriction = 0.0;
		m.staticFriction = 0.0;
		wsm = scene->createMaterial(m);
	}

	NxWheelShapeDesc wheelShapeDesc;

	wheelShapeDesc.localPose.t = wheelDesc->position;
	NxQuat q;
	q.fromAngleAxis(90.0f, NxVec3(0,1,0));
	wheelShapeDesc.localPose.M.fromQuat(q);
	wheelShapeDesc.materialIndex = wsm->getMaterialIndex();
	wheelFlags = wheelDesc->wheelFlags;



	NxReal heightModifier = (wheelDesc->wheelSuspension + wheelDesc->wheelRadius) / wheelDesc->wheelSuspension;

	wheelShapeDesc.suspension.spring = wheelDesc->springRestitution*heightModifier;
	wheelShapeDesc.suspension.damper = wheelDesc->springDamping*heightModifier;
	wheelShapeDesc.suspension.targetValue = wheelDesc->springBias*heightModifier;

	wheelShapeDesc.radius = wheelDesc->wheelRadius;
	wheelShapeDesc.suspensionTravel = wheelDesc->wheelSuspension; 
	wheelShapeDesc.inverseWheelMass = wheelDesc->inverseWheelMass;//0.2f;	//not given!? TODO

	//NxTireFunctionDesc tfd;
	//tfd.stiffnessFactor = 1000; //tony
	//wheelShapeDesc.longitudalTireForceFunction.extremumValue = 0.002;
	//wheelShapeDesc.longitudalTireForceFunction.asymptoteValue = 0.001;

	wheelShapeDesc.lateralTireForceFunction.stiffnessFactor *= wheelDesc->frictionToSide;	
	wheelShapeDesc.longitudalTireForceFunction.stiffnessFactor *= wheelDesc->frictionToFront;	

	wheelShape = static_cast<NxWheelShape *>(actor->createShape(wheelShapeDesc));
}

NxWheel::~NxWheel()
{
}

void NxWheel::tick(NxReal motorTorque, NxReal brakeTorque)
{
	//motorTorque *= 0.1f;
	//brakeTorque *= 500.0f;
	//if(handBrake && getWheelFlag(NX_WF_AFFECTED_BY_HANDBRAKE))
	//	brakeTorque = 1000.0f;

	if(getWheelFlag(NX_WF_ACCELERATED)) 
		wheelShape->setMotorTorque(motorTorque);

	wheelShape->setBrakeTorque(brakeTorque);
}

NxActor* NxWheel::getTouchedActor() const
{
	NxWheelContactData wcd;
	NxShape * s = wheelShape->getContact(wcd);	
	return s ? &s->getActor() : 0;
}

NxVec3 NxWheel::getWheelPos() const
{
	return wheelShape->getLocalPosition();
}

void NxWheel::setAngle(NxReal angle)
{
	wheelShape->setSteerAngle(-angle);
}

NxReal NxWheel::getRpm() const
{
	return NxMath::abs(wheelShape->getAxleSpeed())/NxTwoPi * 60.0f;
}
