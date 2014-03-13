#ifndef NX_WHEEL
#define NX_WHEEL

#include <cstdio>

#include "NxWheelDesc.h"
#include <NxPhysics.h>

class NxWheel
{
private:
	// 只能用createWheel生成实例
	NxWheel(NxActor* actor, NxWheelDesc* wheelDesc);//, Ogre::SceneNode* node);
public:
	virtual	 ~NxWheel();
	// 传进来的参数为其所在actor, wheel描述, 父结点
	static NxWheel* createWheel(NxActor* actor, NxWheelDesc* wheelDesc);//, Ogre::SceneNode* node);

	virtual void setSideSlipEffect(NxTireFunctionDesc tireF)
	{
		wheelShape->setLateralTireForceFunction(tireF);
	}
	virtual void tick(NxReal motorTorque, NxReal brakeTorque);	// 轮子真正产生运动函数
	virtual NxActor* getTouchedActor() const;	// 得到与轮子接近的物体
	virtual NxVec3 getWheelPos() const;	// 得到轮子相对位置
	virtual void setAngle(NxReal angle);	// 设置轮子角度
	virtual NxReal getRpm() const;	// 得到转速

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
	NxU32 wheelFlags;	// 轮子特性的标签

private:
	NxActor* actor;
	NxWheelShape * wheelShape;
};
#endif