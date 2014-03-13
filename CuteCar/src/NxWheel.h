#ifndef NX_WHEEL
#define NX_WHEEL

#include <cstdio>

#include "NxWheelDesc.h"
#include <NxPhysics.h>

class NxWheel
{
private:
	// ֻ����createWheel����ʵ��
	NxWheel(NxActor* actor, NxWheelDesc* wheelDesc);//, Ogre::SceneNode* node);
public:
	virtual	 ~NxWheel();
	// �������Ĳ���Ϊ������actor, wheel����, �����
	static NxWheel* createWheel(NxActor* actor, NxWheelDesc* wheelDesc);//, Ogre::SceneNode* node);

	virtual void setSideSlipEffect(NxTireFunctionDesc tireF)
	{
		wheelShape->setLateralTireForceFunction(tireF);
	}
	virtual void tick(NxReal motorTorque, NxReal brakeTorque);	// �������������˶�����
	virtual NxActor* getTouchedActor() const;	// �õ������ӽӽ�������
	virtual NxVec3 getWheelPos() const;	// �õ��������λ��
	virtual void setAngle(NxReal angle);	// �������ӽǶ�
	virtual NxReal getRpm() const;	// �õ�ת��

	virtual NxVec3 getGroundContactPos() const 
	{
		return getWheelPos()+NxVec3(0, -wheelShape->getRadius(), 0);
	}

	virtual float getRadius() const
	{
		return wheelShape->getRadius();
	}

	NX_INLINE bool hasGroundContact() const 
	{
		return getTouchedActor() != NULL;
	}

	NX_INLINE bool getWheelFlag(NxWheelFlags flag) const
	{ 
		return (wheelFlags & flag) != 0; 
	}

public:
	void* userData;

protected:
	NxU32 wheelFlags;	// �������Եı�ǩ

private:
	NxActor* actor;
	NxWheelShape * wheelShape;
};
#endif