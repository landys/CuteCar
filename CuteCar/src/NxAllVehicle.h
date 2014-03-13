#pragma once

#include "NxVehicle.h"

class NxAllVehicle
{
public:
	static NxAllVehicle* getSingleton();	// 得到唯一的车辆管理的对象

	NxVehicle* addVehicle(const NxVec3& pos, VehicleInfo vinfo, std::string name, NxScene* nxScene, NxPhysicsSDK* nxPhysics);	// 增加一辆车
	NxVehicle* getUserVehicle();	// 得到当前用可操作的车辆的指针
	void killUserVehicle();	// 当前汽车已输
	NxVehicle* changeUserVehicle();	// 转换到下一辆车为用户控制的车.此函数只用来调试用,在游戏中不会使用
	void control(int steering, int acceleration, bool handBrake);
	void standUp();
	void setToBeginning(const NxVec3& pos);
	void updateAllVehicle(NxReal lastTimeStepSize);
	int setCurrentUser(const std::string& name);	// 设置当前用户
	void setToSlip();	//设为打滑效果
	void cancleSlip();	// 无打滑效果
	void setMaxVelocity(float velocity);
	AllVehicleInfoNx* getVehicleInfo();
	int getOilAmount();
	void setOilAmount(int oilAmount);
	int getLoopNum();
	void setLoopNum(int nLoop);
	float getUserVelocity();
	int getNumUser()	// 得到用户数量
	{
		return mAllVehicle.size();
	}
protected:
	NxAllVehicle(void);
	virtual ~NxAllVehicle(void);
private:
	static NxAllVehicle* mSingleton;	// 唯一的类对象

	NxArray<NxVehicle*> mAllVehicle;	// 车辆数组
	NxArray<int> mIsLive;	// 标志相应车辆是否挂了,0,没挂,1-n,挂的顺序,实现排名
	AllVehicleInfoNx* mAllInfoNx;
	int mSizeAllInfoNx;
	int mCountDead;	// 当前挂的车辆的总数
	int miUserVehicle;	// 当前用户车辆下标
};
