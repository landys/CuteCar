#pragma once

#include "NxVehicle.h"

class NxAllVehicle
{
public:
	static NxAllVehicle* getSingleton();	// �õ�Ψһ�ĳ�������Ķ���

	NxVehicle* addVehicle(const NxVec3& pos, VehicleInfo vinfo, std::string name, NxScene* nxScene, NxPhysicsSDK* nxPhysics);	// ����һ����
	NxVehicle* getUserVehicle();	// �õ���ǰ�ÿɲ����ĳ�����ָ��
	void killUserVehicle();	// ��ǰ��������
	NxVehicle* changeUserVehicle();	// ת������һ����Ϊ�û����Ƶĳ�.�˺���ֻ����������,����Ϸ�в���ʹ��
	void control(int steering, int acceleration, bool handBrake);
	void standUp();
	void setToBeginning(const NxVec3& pos);
	void updateAllVehicle(NxReal lastTimeStepSize);
	int setCurrentUser(const std::string& name);	// ���õ�ǰ�û�
	void setToSlip();	//��Ϊ��Ч��
	void cancleSlip();	// �޴�Ч��
	void setMaxVelocity(float velocity);
	AllVehicleInfoNx* getVehicleInfo();
	int getOilAmount();
	void setOilAmount(int oilAmount);
	int getLoopNum();
	void setLoopNum(int nLoop);
	float getUserVelocity();
	int getNumUser()	// �õ��û�����
	{
		return mAllVehicle.size();
	}
protected:
	NxAllVehicle(void);
	virtual ~NxAllVehicle(void);
private:
	static NxAllVehicle* mSingleton;	// Ψһ�������

	NxArray<NxVehicle*> mAllVehicle;	// ��������
	NxArray<int> mIsLive;	// ��־��Ӧ�����Ƿ����,0,û��,1-n,�ҵ�˳��,ʵ������
	AllVehicleInfoNx* mAllInfoNx;
	int mSizeAllInfoNx;
	int mCountDead;	// ��ǰ�ҵĳ���������
	int miUserVehicle;	// ��ǰ�û������±�
};
