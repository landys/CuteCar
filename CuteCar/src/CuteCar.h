#include "LoadLibs.h"
#include "NxTerrain.h"
#include "OgreTerrain.h"
#include <OgreNoMemoryMacros.h> 
#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>

#include <CEGUI/elements/CEGUIStaticImage.h>
#include <CEGUI/elements/CEGUIStaticText.h>
#include <CEGUI/elements/CEGUIPushButton.h>

#include "OgreCEGUIRenderer.h"
#include "OgreCEGUIResourceProvider.h"

#include "ExampleApplication.h"
#include <OgreTimer.h>

#include "AudioClass.h"

#include "string.h"
#include <map>

// 游戏模式: 油量模式和圈数模式
enum GameMode {OIL_MODE, LOOP_MODE};

// 转换类
class NxOgreConvertor
{
public:
	static Ogre::Vector3 toOgre(const NxVec3 &v) 
	{
		return Ogre::Vector3(v.x, v.y, v.z);
	}

	static Ogre::Quaternion toOgre(const NxQuat &q) 
	{
		return Ogre::Quaternion(q.w, q.x, q.y, q.z);
	}

	static AllVehicleInfoOgre toOgre(const AllVehicleInfoNx& allvn)
	{
		AllVehicleInfoOgre allvo;
		allvo.vepos = toOgre(allvn.vepos);
		allvo.verot = toOgre(allvn.verot);
		allvo.campos = toOgre(allvn.campos);
		allvo.camrot = toOgre(allvn.camrot);
		allvo.angle0 = allvn.angle0;
		allvo.angle1 = allvn.angle1;
		allvo.angle2 = allvn.angle2;
		allvo.angle3 = allvn.angle3;
		allvo.name = allvn.name;

		return allvo;
	}

	static NxVec3 toNx(const Ogre::Vector3 &v) 
	{
		return NxVec3(v.x, v.y, v.z);
	}

	static NxRay toNx(const Ogre::Ray &ray) 
	{
		return NxRay(toNx(ray.getOrigin()), toNx(ray.getDirection()));
	}

	static NxMat33 &toNx(Ogre::Matrix3 &m) 
	{
		return *(NxMat33*)&m;
	}
};

CEGUI::MouseButton convertOgreButtonToCegui(int buttonID)
{
	switch (buttonID)
	{
	case MouseEvent::BUTTON0_MASK:
		return CEGUI::LeftButton;
	case MouseEvent::BUTTON1_MASK:
		return CEGUI::RightButton;
	case MouseEvent::BUTTON2_MASK:
		return CEGUI::MiddleButton;
	case MouseEvent::BUTTON3_MASK:
		return CEGUI::X1Button;
	default:
		return CEGUI::LeftButton;
	}
}

class CuteCarFrameListener : public ExampleFrameListener, public MouseMotionListener, public MouseListener
{
protected:
	SceneManager* mSceneMgr;   // The current SceneManager
	NxTerrain* mNx;	// 操纵物理引擎的接口
	OgreTerrain* mOgre;	// 操纵OGRE的接口

	GameMode mGameMode;	// 游戏模式
	int mLoopNum;	// 游戏总圈数

	bool m_IsClosed;
	int m_Acceleration;
	int m_Steering;
	bool m_HandBrake;
	bool m_Paused;
	bool mIsReset;
	bool mIsMain;

	// for CEGUI
	bool mPlaying;
	CEGUI::String mUsrName;
	CEGUI::Renderer* mGUIRenderer;
	CEGUI::System* mGUISystem;	

	CEGUI::Window* mLoginSheet;
	CEGUI::Window* mHelpSheet;
	CEGUI::Window* mPlaceSheet;
	CEGUI::Window* mEndSheet;
	CEGUI::Window* mPlaySheet;
	CEGUI::Window* mEditBox;

	CEGUI::StaticText* mSpeed;
	CEGUI::StaticText* mLoop;
	CEGUI::StaticText* mTime;
	CEGUI::StaticText* mTip;
	CEGUI::StaticImage* mMeter;
	CEGUI::StaticImage* mTools[6];
	CEGUI::Imageset* mIs;

	CEGUI::StaticImage* mCheckEsc;
	CEGUI::Imageset* mIsele;
	CEGUI::StaticImage* mMapPlace;

	Timer mMytime;
	int mCount;
	int mTimeCount;
	int mMillCount;
	int mCurSpeed;	// 当前显示在计表盘上的时间

	int mTimeGet;
	int mTimeGetOld;
	int mTimeOilOld;
	int mTimeItemOld;

	int mTimeSlipOld;
	int mTimeSpeedOld;
	int mTimeTextOld;

	bool mIsUserCamera;

	int mNumPressed;

	// 声音实现
	AudioClass* mAudi;
	SoundClass* mSound;
	SoundClass* mSoundUseItem;
	SoundClass* mSoundGetItem;
	SoundClass* mSoundBegin;
	SoundClass* mSoundEnd;

	bool mIsSound;
	bool mIsSoundUseItem;
	bool mIsSoundGetItem;
	bool mIsSoundAcc;

	// 道具位置
	Vector3 mRandPlace[14];
	int mIsPlaced[14];	// 标志有没放道具

public:
	CuteCarFrameListener(RenderWindow* win, Camera* cam, SceneManager* sceneMgr)
		: ExampleFrameListener(win, cam, true), mNx(NULL), mOgre(NULL)
	{
		INITLOG;
		mRandPlace[0] = Vector3(158.947, 2.42724, -292.06); 
		mRandPlace[1] = Vector3(206.531, 2.42722, -324.621); 
		mRandPlace[2] = Vector3(259.949, 2.42722, -292.177); 
		mRandPlace[3] = Vector3(290.579, 2.42722, -253.848); 
		mRandPlace[4] = Vector3(218.998, 2.42722, -233.136);
		mRandPlace[5] = Vector3(190.548, 2.42722, -153.747); 
		mRandPlace[6] = Vector3(245.896, 2.42722, -149.736);
		mRandPlace[7] = Vector3(285.027, 2.42722, -135.517); 
		mRandPlace[8] = Vector3(293.291, 2.42722, -70.2195);
		mRandPlace[9] = Vector3(241.401, 2.42722, -77.1035); 
		mRandPlace[10] = Vector3(112.947, 2.42722, -107.711);
		mRandPlace[11] = Vector3(70.5575, 2.42722, -110.134);
		mRandPlace[12] = Vector3(176.594, 2.42722, -58.393); 
		mRandPlace[13] = Vector3(246.805, 2.42722, -123.317);

		mSceneMgr = sceneMgr;

		mEventProcessor->addMouseListener(this);
		mEventProcessor->addMouseMotionListener(this);

		// setup GUI system
		mGUIRenderer = new CEGUI::OgreCEGUIRenderer(win, 
			Ogre::RENDER_QUEUE_OVERLAY, false, 3000, sceneMgr);
		mGUISystem = new CEGUI::System(mGUIRenderer);

		CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);
		// load scheme and set up defaults
		CEGUI::SchemeManager::getSingleton().loadScheme(
			(CEGUI::utf8*)"GUIConfigSkin.scheme");		
		mGUISystem->setDefaultMouseCursor(
			(CEGUI::utf8*)"GUIConfig", (CEGUI::utf8*)"MouseArrow");
		mGUISystem->setDefaultFont((CEGUI::utf8*)"Tahoma-12");

		mNx = new NxTerrain();
		mNx->initNx();	// 初始化物理引擎
		mNx->createNxTerrain();
		mOgre = new OgreTerrain(mSceneMgr);
		mOgre->createOgreTerrain();
		mOgre->addScratchLine();	// 添加起跑线

		Vector3 pos(26*SCALE_TERRAIN, 2*SCALE_TERRAIN, -68*SCALE_TERRAIN);

		VehicleInfo vinfo;
		vinfo.width = 2.3058;
		vinfo.height = 1.2054;
		vinfo.length = 5.0556;
		vinfo.mass = 600;
		vinfo.steerablity = 0.011f;
		vinfo.maxSteeringAngle = 10.f;
		vinfo.maxAcceleraion = 7.f;
		vinfo.driven = FrontDriven;
		vinfo.maxHandBraking = 1000;
		vinfo.maxVelocity = 65.f;
		vinfo.type = Car;
		vinfo.oilAmount = 200;

		mNx->addUser(NxOgreConvertor::toNx(pos), vinfo, "tony");
		mNx->userControl("tony");
		mNx->initTerrainItem(mNx->getNxScene());	// 必须先初始化
		mNx->addScratchLine();	// 加入起跑线

		mOgre->addUser("tony", pos);
		mOgre->userCamera(mCamera, "tony");
		
		for (int i=0; i<14; i++)
		{
			mIsPlaced[i] = -1;
		}

		// 声音效果
		mAudi = new AudioClass(GetForegroundWindow());
		mAudi->Initialize();
		mAudi->SetDefer(false);
		mAudi->SetListenerPosition(0,0,500.0);
		mAudi->SetListenerOrientFront(0,0,-1.0);
		mAudi->SetListenerOrientTop(0,1.0,0);
		mAudi->SetListenerParameters(1.0,1.0);

		mSound = new SoundClass(mAudi);
		mSound->CreateBuffer(SOUND_TRAVEL_FILE);
		mSound->SetPosition(0,0,0);
		mSound->SetConeOrientation(0,0,1);
		mSound->SetInsideConeAngle(40);
		mSound->SetOutsideConeAngle(90);
		long lvolumn = 0;
		mSound->GetBuffer()->GetVolume(&lvolumn);
		mSound->SetConeOutsideVolume(lvolumn*0.5);
		mSound->SetMaxDistance(1000.0);
		mSound->SetMinDistance(20.0);
		mSound->SetLoop(true);

		mSoundUseItem = new SoundClass(mAudi);
		mSoundUseItem->CreateBuffer(SOUND_USE_ITEM_FILE);
		mSoundUseItem->SetPosition(0,0,0);
		mSoundUseItem->SetConeOrientation(0,0,1);
		mSoundUseItem->SetInsideConeAngle(40);
		mSoundUseItem->SetOutsideConeAngle(90);
		lvolumn = 0;
		mSoundUseItem->GetBuffer()->GetVolume(&lvolumn);
		mSoundUseItem->SetConeOutsideVolume(lvolumn*0.5);
		mSoundUseItem->SetMaxDistance(1000.0);
		mSoundUseItem->SetMinDistance(20.0);
		mSound->SetLoop(false);

		mSoundGetItem = new SoundClass(mAudi);
		mSoundGetItem->CreateBuffer(SOUND_GET_ITEM_FILE);

		mSoundGetItem->SetPosition(0,0,0);
		mSoundGetItem->SetConeOrientation(0,0,1);
		mSoundGetItem->SetInsideConeAngle(40);
		mSoundGetItem->SetOutsideConeAngle(90);
		lvolumn = 0;
		mSoundGetItem->GetBuffer()->GetVolume(&lvolumn);
		mSoundGetItem->SetConeOutsideVolume(lvolumn*0.5);
		mSoundGetItem->SetMaxDistance(1000.0);
		mSoundGetItem->SetMinDistance(20.0);
		mSound->SetLoop(false);

		mSoundBegin = new SoundClass(mAudi);
		mSoundBegin->CreateBuffer(SOUND_BEGIN_FILE);

		mSoundBegin->SetPosition(0,0,0);
		mSoundBegin->SetConeOrientation(0,0,1);
		mSoundBegin->SetInsideConeAngle(40);
		mSoundBegin->SetOutsideConeAngle(90);
		lvolumn = 0;
		mSoundBegin->GetBuffer()->GetVolume(&lvolumn);
		mSoundBegin->SetConeOutsideVolume(lvolumn*0.5);
		mSoundBegin->SetMaxDistance(1000.0);
		mSoundBegin->SetMinDistance(20.0);
		mSound->SetLoop(false);

		mSoundEnd = new SoundClass(mAudi);
		mSoundEnd->CreateBuffer(SOUND_END_FILE);

		mSoundEnd->SetPosition(0,0,0);
		mSoundEnd->SetConeOrientation(0,0,1);
		mSoundEnd->SetInsideConeAngle(40);
		mSoundEnd->SetOutsideConeAngle(90);
		lvolumn = 0;
		mSoundEnd->GetBuffer()->GetVolume(&lvolumn);
		mSoundEnd->SetConeOutsideVolume(lvolumn*0.5);
		mSoundEnd->SetMaxDistance(1000.0);
		mSoundEnd->SetMinDistance(20.0);
		mSound->SetLoop(false);

		mSoundBegin->Play();
		loadMain();
	}

	~CuteCarFrameListener()
	{
		if (mNx)
		{
			mNx->releasePhysics();
			delete mNx;
		}
		if (mOgre)
		{
			delete mOgre;
		}

		if(mGUISystem)
		{
			delete mGUISystem;
			mGUISystem = 0;
		}
		if(mGUIRenderer)
		{
			delete mGUIRenderer;
			mGUIRenderer = 0;
		}
	}

	void writeGrade(void)
	{
		int i = 0;
		int n = 0;
 		int ilength = 0;

		int numLap = 0;
		int lengthLap[10];
		char nameLap[10][20];
		int scoreLap[10];

		int numOil = 0;
		int lengthOil[10];
		char nameOil[10][20];
		int scoreOil[10];

		FILE* fin = fopen(SCORE_FILE, "rb");
		if (fin != NULL)
		{
			fread(&numLap, sizeof(int), 1, fin);
			for (i=0; i<numLap; i++)
			{
				fread(&scoreLap[i], sizeof(int), 1, fin);
				fread(&lengthLap[i], sizeof(int), 1, fin);
				fread(nameLap[i], sizeof(char), lengthLap[i], fin);
				nameLap[i][lengthLap[i]] = '\0';
			}
			fread(&numOil, sizeof(int), 1, fin);
			for (i=0; i<numOil; i++)
			{
				fread(&scoreOil[i], sizeof(int), 1, fin);
				fread(&lengthOil[i], sizeof(int), 1, fin);
				fread(nameOil[i], sizeof(char), lengthOil[i], fin);
				nameOil[i][lengthOil[i]] = '\0';
			}
			fclose(fin);
		}

		FILE* fout = fopen(SCORE_FILE, "wb");
		if (fout == NULL)
		{
			return;
		}
		if (mGameMode == LOOP_MODE)
		{
			if (++numLap > 10)
			{
				numLap = 10;
			}
			fwrite(&numLap, sizeof(int), 1, fout);
			for (i=0; i<numLap; i++)
			{
				if (mTimeCount < scoreLap[i])
				{
					fwrite(&mTimeCount, sizeof(int), 1, fout);
					if (mUsrName == "")
					{
						mUsrName = "God";
					}
					ilength = mUsrName.length();
					if (ilength > 10)
					{
						ilength = 10;
					}
					fwrite(&ilength, sizeof(int), 1, fout);
					fwrite(mUsrName.c_str(), sizeof(char), ilength, fout);

					n = numLap - 1;
					break;
				}
				else
				{
					fwrite(&scoreLap[i], sizeof(int), 1, fout);
					fwrite(&lengthLap[i], sizeof(int), 1, fout);
					fwrite(nameLap[i], sizeof(char), lengthLap[i], fout);
				}
			}
			for (; i<n; i++)
			{
				fwrite(&scoreLap[i], sizeof(int), 1, fout);
				fwrite(&lengthLap[i], sizeof(int), 1, fout);
				fwrite(nameLap[i], sizeof(char), lengthLap[i], fout);
			}

			fwrite(&numOil, sizeof(int), 1, fout);
			for (i=0; i<numOil; i++)
			{
				fwrite(&scoreOil[i], sizeof(int), 1, fout);
				fwrite(&lengthOil[i], sizeof(int), 1, fout);
				fwrite(nameOil[i], sizeof(char), lengthOil[i], fout);
			}
			fclose(fout);
		}
		else if (mGameMode == OIL_MODE)
		{
			fwrite(&numLap, sizeof(int), 1, fout);
			for (i=0; i<numLap; i++)
			{
				fwrite(&scoreLap[i], sizeof(int), 1, fout);
				fwrite(&lengthLap[i], sizeof(int), 1, fout);
				fwrite(nameLap[i], sizeof(char), lengthLap[i], fout);
			}

			if (++numOil > 10)
			{
				numOil = 10;
			}
			fwrite(&numOil, sizeof(int), 1, fout);
			for (i=0; i<numOil; i++)
			{
				if (mTimeCount > scoreOil[i])
				{
					fwrite(&mTimeCount, sizeof(int), 1, fout);
					if (mUsrName == "")
					{
						mUsrName = "God";
					}
					ilength = mUsrName.length();
					if (ilength > 10)
					{
						ilength = 10;
					}
					fwrite(&ilength, sizeof(int), 1, fout);
					fwrite(mUsrName.c_str(), sizeof(char), ilength, fout);

					n = numOil - 1;
					break;
				}
				else
				{
					fwrite(&scoreOil[i], sizeof(int), 1, fout);
					fwrite(&lengthOil[i], sizeof(int), 1, fout);
					fwrite(nameOil[i], sizeof(char), lengthOil[i], fout);
				}
			}
			for (; i<n; i++)
			{
				fwrite(&scoreOil[i], sizeof(int), 1, fout);
				fwrite(&lengthOil[i], sizeof(int), 1, fout);
				fwrite(nameOil[i], sizeof(char), lengthOil[i], fout);
			}

			fclose(fout);
		}
	}

	String i2TimeString(int n)	// 以十毫秒为单位
	{
		int sec = n / 100;
		int mini = n % 100;
		int min = sec / 60;
		sec = sec % 60;
		char out[15];
		sprintf(out, "%d%d:%d%d:%d%d", min/10, min%10, sec/10, sec%10, mini/10, mini%10);
		return String(out);
	}

	void closesheet()
	{
		INITLOG;
		if(mLoginSheet!=NULL)
		{
			CEGUI::WindowManager::getSingleton().destroyWindow(mLoginSheet);
			mLoginSheet = NULL;
		}
		if(mHelpSheet!=NULL)
		{
			CEGUI::WindowManager::getSingleton().destroyWindow(mHelpSheet);
			mHelpSheet = NULL;
		}
		if(mPlaceSheet!=NULL)
		{
			CEGUI::WindowManager::getSingleton().destroyWindow(mPlaceSheet);
			mPlaceSheet = NULL;
		}
		if(mEndSheet!=NULL)
		{
			CEGUI::WindowManager::getSingleton().destroyWindow(mEndSheet);
			mEndSheet = NULL;
		}
		if(mPlaySheet!=NULL)
		{			
			mPlaying = false;
			CEGUI::WindowManager::getSingleton().destroyWindow(mPlaySheet);
			mPlaySheet = NULL;		
		}
	}

	void setupEventHandlers(void)
	{
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		mEditBox = wmgr.getWindow((CEGUI::utf8*)"UsrName");
		wmgr.getWindow((CEGUI::utf8*)"SpeedMode")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::ModeSpeed, this));
		wmgr.getWindow((CEGUI::utf8*)"DeathMode")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::ModeDeath, this));
		wmgr.getWindow((CEGUI::utf8*)"Help")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handleHelp, this));
		wmgr.getWindow((CEGUI::utf8*)"Place")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handlePlace, this));
		wmgr.getWindow((CEGUI::utf8*)"Quit")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handleQuit, this));
		CEGUI::Window* root = wmgr.getWindow("Login");
	}

	void loadMain()
	{		
		closesheet();
		mLoginSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"login.layout");	
		mGUISystem->setGUISheet(mLoginSheet);
		setupEventHandlers();
		mIsMain = true;
	}

	void playscheme()
	{
		INITLOG;
		
		closesheet();
		mPlaySheet = CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"GUIConfig.layout");		
		mGUISystem->setGUISheet(mPlaySheet);

		mNx->setToBeginning(NxVec3(26*SCALE_TERRAIN, 2*SCALE_TERRAIN, -68*SCALE_TERRAIN));
		mOgre->setToBeginning("tony");
		mOgre->userCamera(mCamera, "tony");
		//mNx->setLoopNum(0);

		mLoopNum = 4;

		mPlaying = false;
		//mUsrName = "";

		m_IsClosed = false;
		m_Paused = false;

		m_Acceleration = 0;
		m_Steering = 0;
		m_HandBrake = false;

		mIsReset = false;

		mIs = CEGUI::ImagesetManager::getSingleton().getImageset((CEGUI::utf8*)"meter");

		mIsele = CEGUI::ImagesetManager::getSingleton().getImageset((CEGUI::utf8*)"elements");
		CEGUI::StaticImage* info = static_cast<CEGUI::StaticImage*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Info"));
		mMapPlace = static_cast<CEGUI::StaticImage*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"self"));
		mMapPlace->setPosition(CEGUI::Point(0.185714, 0.485714));
		if (mGameMode == OIL_MODE)
		{
			info->setImage(&mIsele-> getImage((CEGUI::utf8*)"infooil"));
		}
		else if (mGameMode == LOOP_MODE)
		{
			info->setImage(&mIsele-> getImage((CEGUI::utf8*)"infolap"));
		}

		for(int k = 0; k < 6; k++)
		{
			mTools[k] = static_cast<CEGUI::StaticImage*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)("Tool"+StringConverter::toString(k)).c_str()));
		}

		mMeter = static_cast<CEGUI::StaticImage*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Meter"));
		mMeter->setImage(&mIs-> getImage((CEGUI::utf8*)"1"));	

		mTip = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Tip"));
		mTip->setText((CEGUI::utf8*)"");
		mSpeed = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Speed"));
		mLoop = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Loop"));
		mTime = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Time"));
		if (mGameMode == OIL_MODE)
		{
			mLoop->setText(StringConverter::toString(mNx->getOilAmount()));
		}
		else if (mGameMode == LOOP_MODE)
		{
			mLoop->setText(StringConverter::toString(mNx->getLoopNum()) + "/" + StringConverter::toString(mLoopNum));
		}
		
		mCheckEsc =	static_cast<CEGUI::StaticImage*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"CheckEsc"));
		mCheckEsc->hide();

		mCount = 0;
		mTimeCount = 0;
		mMillCount = 0;
		mCurSpeed = 0;
		setSpeed(mCount);
		setTime(mTimeCount);
		mPlaying = true;

		mTimeGet = 0;
		mTimeGetOld = 0;
		mTimeOilOld = 0;
		mTimeItemOld = 0;

		mTimeSlipOld = 2000000000;
		mTimeSpeedOld = 2000000000;
		mTimeTextOld = 2000000000;

		mNumPressed = 0;

		mIsSound = false;
		mIsSoundUseItem = false;
		mIsSoundGetItem = false;
		mIsSoundAcc = false;

		mIsUserCamera = true;
		mMytime.reset();
	}

	bool ModeSpeed(const CEGUI::EventArgs& e)
	{
		mIsMain = false;
		mUsrName = mEditBox->getText();
		mGameMode = LOOP_MODE;
		playscheme();
		return true;
	}

	bool ModeDeath(const CEGUI::EventArgs& e)
	{
		mIsMain = false;
		mUsrName = mEditBox->getText();
		mGameMode = OIL_MODE;
		playscheme();
		return true;
	}

	bool handleHelp(const CEGUI::EventArgs& e)
	{
		mIsMain = false;
		closesheet();
		mHelpSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"help.layout");
		mGUISystem->setGUISheet(mHelpSheet);
		return true;
	}

	bool handlePlace(const CEGUI::EventArgs& e)
	{
		mIsMain = false;
		closesheet();
		mPlaceSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"place.layout");
		mGUISystem->setGUISheet(mPlaceSheet);

		int i = 0;

		int numLap = 0;
		int lengthLap[10];
		char nameLap[10][20];
		int scoreLap[10];

		int numOil = 0;
		int lengthOil[10];
		char nameOil[10][20];
		int scoreOil[10];

		FILE* fin = fopen(SCORE_FILE, "rb");
		if (fin != NULL)
		{
			fread(&numLap, sizeof(int), 1, fin);
			for (i=0; i<numLap; i++)
			{
				fread(&scoreLap[i], sizeof(int), 1, fin);
				fread(&lengthLap[i], sizeof(int), 1, fin);
				fread(nameLap[i], sizeof(char), lengthLap[i], fin);
				nameLap[i][lengthLap[i]] = '\0';
			}
			fread(&numOil, sizeof(int), 1, fin);
			for (i=0; i<numOil; i++)
			{
				fread(&scoreOil[i], sizeof(int), 1, fin);
				fread(&lengthOil[i], sizeof(int), 1, fin);
				fread(nameOil[i], sizeof(char), lengthOil[i], fin);
				nameOil[i][lengthOil[i]] = '\0';
			}
			fclose(fin);
		}
		else
		{
			return false;
		}

		CEGUI::StaticText* race[10];
		CEGUI::StaticText* racetime[10];
		CEGUI::StaticText* dead[10];
		CEGUI::StaticText* deadtime[10];
		for(int i = 0; i < 10; i++)
		{
			race[i] = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)("Race"+StringConverter::toString(i)).c_str()));
			racetime[i] = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)("RaceTime"+StringConverter::toString(i)).c_str()));
			dead[i] = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)("Dead"+StringConverter::toString(i)).c_str()));
			deadtime[i] = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)("DeadTime"+StringConverter::toString(i)).c_str()));
			race[i]->setText(nameLap[i]);
			dead[i]->setText(nameOil[i]);
			racetime[i]->setText(i2TimeString(scoreLap[i]));
			deadtime[i]->setText(i2TimeString(scoreOil[i]));
		}
		return true;
	}

	bool handleQuit(const CEGUI::EventArgs& e)
	{
		m_IsClosed = true;
		return true;
	}

	bool handleReturn(const CEGUI::EventArgs& e)
	{
		loadMain();
		return true;
	}

	void GameOver()
	{
		mSoundEnd->Play();
		mIsMain = false;
		mPlaying = false;

		writeGrade();	// 写入记录文件
		closesheet();
		
		mEndSheet = CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"over.layout");
		mGUISystem->setGUISheet(mEndSheet);
		CEGUI::StaticText* grade = static_cast<CEGUI::StaticText*>(CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"Grade"));
		grade->setText(i2TimeString(mTimeCount));
		setupGameOverEvent();
	}

	void setupGameOverEvent()
	{
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		wmgr.getWindow((CEGUI::utf8*)"ShowGrade")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handlePlace, this));
		wmgr.getWindow((CEGUI::utf8*)"Return")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handleReturn, this));
		wmgr.getWindow((CEGUI::utf8*)"Quit")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handleQuit, this));
	}

	void checkESC()
	{
		if (mCheckEsc) 
		{				
			mCheckEsc->show();			
			setupCheckHandler();
		}
		else
		{
			loadMain();
		}
	}

	void setupCheckHandler()
	{
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		wmgr.getWindow((CEGUI::utf8*)"Yes")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handleChecked, this));
		wmgr.getWindow((CEGUI::utf8*)"No")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&CuteCarFrameListener::handleResume, this));
	}
	bool handleChecked(const CEGUI::EventArgs& e)
	{
		mPlaying = false;
		mCheckEsc->hide();
		//CEGUI::WindowManager::getSingleton().destroyWindow(mCheckEsc);			
		/////////////////////////////////
		loadMain();
		return true;
	}
	bool handleResume(const CEGUI::EventArgs& e)
	{
		if (mCheckEsc) 
		{
			mCheckEsc->hide();
		}
		mPlaying = true;
		m_Paused = false;
		return true;
	}

	char* getStr()
	{
		char* ret = new char[3];
		sprintf(ret, "%d", mCount+1);
		return ret;
	}

	void setTime(int itimecount)
	{
		int sec = itimecount / 100;
		int mini = itimecount % 100;
		int min = sec / 60;
		sec = sec % 60;
		char out[15];
		sprintf(out, "%d%d:%d%d:%d%d", min/10, min%10, sec/10, sec%10, mini/10, mini%10);
		mTime->setText((CEGUI::utf8*)out);
	}

	void setSpeed(int speed)
	{
		char out[10];
		sprintf(out, "%3d km/h", speed);
		mSpeed->setText((CEGUI::utf8*)out);
	}


	bool frameStarted(const FrameEvent &evt)
	{	
		if (mPlaying)
		{
			int forRand = 0;
			if (!m_Paused)
			{				
				if (mNx->getVelocity() > 0 && !mIsSound)
				{
					mSound->Play();
				}
				else
				{
					mSound->Stop();
				}

				if (mGameMode == LOOP_MODE && mNx->getLoopNum() == 5)
				{
					GameOver();
				}
				else if (mGameMode == OIL_MODE && mNx->getOilAmount() == 0)
				{
					GameOver();
				}

				mNx->simulate(evt.timeSinceLastFrame, m_Steering, m_Acceleration, m_HandBrake);
				if (mIsReset)
				{
					mNx->standUp();
				}

				mIsReset = false;

				AllVehicleInfoNx* alln = mNx->getAllVehicleInfo();
				int n = mNx->getNumUser();
				AllVehicleInfoOgre* allo = new AllVehicleInfoOgre[n];
				int i;
				for (i=0; i<n; i++)
				{
					allo[i] = NxOgreConvertor::toOgre(alln[i]);
				}
				mOgre->updateAllOgre(allo);

				if (allo[0].vepos.y < -1)
				{
					mNx->standUp();
					mNx->standUp();
				}
				// 小地图
				forRand = (int)(Ogre::Math::Abs(allo[0].vepos.x));
				mMapPlace->setPosition(CEGUI::Point(Ogre::Math::Abs(allo[0].vepos.x/(140*SCALE_TERRAIN)), Ogre::Math::Abs(allo[0].vepos.z/(140*SCALE_TERRAIN))));
				delete[] allo;

				// 道具
				if (mNumPressed != 0)
				{
					std::vector<ItemType>* vvi = mOgre->getAllItem("tony");
					if (vvi->size() >= mNumPressed)
					{
						mSoundUseItem->Play();
						String ss = "";
						switch((*vvi)[mNumPressed-1])
						{
						case ITEM_TYPE_BIGOIL:
							{
								ss = "Used a big fix item, oil increased by 100.";
								int n = mNx->getOilAmount() + 100;
								if (n > 200)
								{
									n = 200;
								}
								mNx->setOilAmount(n);	
								break;
							}
						case ITEM_TYPE_SMALLOIL:
							{
								ss = "Used a big fix item, oil increased by 40.";
								int n = mNx->getOilAmount() + 40;
								if (n > 200)
								{
									n = 200;
								}
								mNx->setOilAmount(n);	
								break;
							}
						case ITEM_TYPE_MIDOIL:
							{
								ss = "Used a big fix item, oil increased by 70.";
								int n = mNx->getOilAmount() + 70;
								if (n > 200)
								{
									n = 200;
								}
								mNx->setOilAmount(n);	
								break;
							}
						case ITEM_TYPE_SPEEDUP:
							ss = "Used a speed item, car speed up for a while.";
							mNx->setMaxVelocity(95);
							mTimeSpeedOld = mTimeGet;
							break;
						}
						mTip->setText((CEGUI::utf8*)ss.c_str());
						mTimeTextOld = mTimeGet;
						int tempi = 0;
						for (std::vector<ItemType>::iterator iter=(*vvi).begin(); iter!=(*vvi).end(); iter++)
						{
							if (++tempi == mNumPressed)
							{
								(*vvi).erase(iter);
								break;
							}
						}
					}
					
					mNumPressed = 0;
				}

				NxArray<ItemPicked*>* pItemPicked = mNx->getItemPicked();
				int m = pItemPicked->size();
				if (m != 0)
				{
					mSoundGetItem->Play();
				}
			
				for (int j=0; j<m; j++)
				{
					//PRINTLOG((*pItemPicked)[j]->id);
					mOgre->removeTerrainItem((*pItemPicked)[j]->id);
					mOgre->addVehicleItem((*pItemPicked)[j]->type, "tony");
					for (int jj=0; jj<14; jj++)
					{
						if (mIsPlaced[jj] == (*pItemPicked)[j]->id)
						{
							mIsPlaced[jj] = -1;
						}
					}

					String test = "";
					
					switch((*pItemPicked)[j]->type)
					{
					case ITEM_TYPE_SLIP:
						test = "Come into a slip item, car to slip for a while.";
						mNx->useSlipProp();
						mTimeSlipOld = mTimeGet;
						break;
					case ITEM_TYPE_BIGOIL:
						test = "Picked a big fix item.";
						break;
					case ITEM_TYPE_MIDOIL:
						test = "Picked a middle fix item.";
						break;
					case ITEM_TYPE_SMALLOIL:
						test = "Picked a small fix item.";
						break;
					case ITEM_TYPE_SPEEDUP:
						test = "Picked a speedup item.";
						break;
					case ITEM_TYPE_BLOCK:
						test = "Come into a block item, car speed down for a while.";
						mNx->setMaxVelocity(40);
						mTimeSpeedOld = mTimeGet;
						break;
					case ITEM_TYPE_ILL:
						{
							test = "Come into a ill item, oil decreased by 40.";
							int n = mNx->getOilAmount() - 40;
							if (n < 0)
							{
								n = 0;
							}
							mNx->setOilAmount(n);	
							break;
						}
						
					}
					mTimeTextOld = mTimeGet;
					mTip->setText((CEGUI::utf8*)test.c_str());
				}
				pItemPicked->clear();
				mOgre->turnAllTerrainItem(0.08);
				std::vector<ItemType>* vvi = mOgre->getAllItem("tony");
				for (i=0; i<vvi->size(); i++)
				{
					switch ((*vvi)[i])
					{
					case ITEM_TYPE_BIGOIL:
						mTools[i]->setImage(&mIsele-> getImage((CEGUI::utf8*)"oill"));
						break;
					case ITEM_TYPE_SMALLOIL:
						mTools[i]->setImage(&mIsele-> getImage((CEGUI::utf8*)"oils"));
						break;
					case ITEM_TYPE_MIDOIL:
						mTools[i]->setImage(&mIsele-> getImage((CEGUI::utf8*)"oilm"));
						break;
					case ITEM_TYPE_SPEEDUP:
						mTools[i]->setImage(&mIsele-> getImage((CEGUI::utf8*)"acc"));
						break;
					}
				}
				for (; i<6; i++)
				{
					mTools[i]->setImage(&mIsele-> getImage((CEGUI::utf8*)"null"));
				}

				// 最高速度为100,正常65左右,显示给用户看的最高时速为250,所以要乘以系数2.5
				// 刻度40格,所以每格2.5
				// CEGUI
				int velocity = mNx->getVelocity();
				mCurSpeed = int(velocity/2.5);
				if (mCurSpeed > mCount)
				{
					mCount++;
				}
				else if (mCurSpeed < mCount)
				{
					mCount--;
				}
				if (mCount < 0)
				{
					mCount = 0;
				}
				else if (mCount > 39)
				{
					mCount = 39;
				}

				mMeter->setImage(&mIs->getImage((CEGUI::utf8*)getStr()));	
				setSpeed(int(velocity*2.5));
				
			}

			mTimeGet = mMytime.getMillisecondsCPU();

			if (mGameMode == OIL_MODE)
			{
				if (mTimeGet - mTimeOilOld - 200 >= 0)
				{
					if (!m_Paused)
					{
						int noil = mNx->getOilAmount();
						if (noil - 1 > 0)
						{
							mNx->setOilAmount(noil-1);
						}
						else
						{
							mNx->setOilAmount(0);
						}
						mLoop->setText(StringConverter::toString(mNx->getOilAmount()));
					}
					
					mTimeOilOld += 200;
				}
			}
			else if (mGameMode == LOOP_MODE)
			{
				int nloop = mNx->getLoopNum();
				if (nloop > mLoopNum)
				{
					nloop = mLoopNum;
				}
				mLoop->setText(StringConverter::toString(nloop) + "/" + StringConverter::toString(mLoopNum));
			}

			if (mTimeGet - mTimeTextOld - 4000 >= 0)
			{
				if (!m_Paused)
				{
					mTip->setText((CEGUI::utf8*)"");
					mTimeTextOld = 2000000000;
				}
				mTimeTextOld += 4000;
			}

			if (mTimeGet - mTimeSlipOld - 5000 >= 0)
			{
				if (!m_Paused)
				{
					mTip->setText((CEGUI::utf8*)("The slip effect has disappeared."));
					mTimeTextOld = mTimeGet;
					mNx->cancelSlipProp();
					mTimeSlipOld = 2000000000;
				}
				mTimeSlipOld += 5000;
			}

			if (mTimeGet - mTimeSpeedOld - 8000 >= 0)
			{
				if (!m_Paused)
				{
					mTip->setText((CEGUI::utf8*)("The speed effect has disappeared."));
					mTimeTextOld = mTimeGet;
					mNx->setMaxVelocity(65);
					mTimeSpeedOld = 2000000000;
				}
				mTimeSpeedOld += 8000;
			}

			if (mTimeGet - mTimeItemOld - 5000 >= 0)
			{
				if (!m_Paused)
				{
					int place = NxMath::rand(0, 300+mTimeGet%500);
					place = (place + forRand) % 14;
					if (mIsPlaced[place] != -1)
					{
						int pp = place;
						pp++;
						if (place == 13)
						{
							pp = 0;
						}
						while (mIsPlaced[pp] != -1 && pp != place)
						{
							pp++;
							if (place == 13)
							{
								pp = 0;
							}
						}
						if (pp == place)
						{
							place = -1;
						}
						else
						{
							place = pp;
						}
					}
					
					if (place != -1)
					{
						int type = NxMath::rand(1, 200+place+mTimeGet%200);
						type = (type + forRand) % 7 + 1;
						NxTerrainItem* ti = mNx->createTerrainItem(ItemType(ITEM_NONE+type), NxOgreConvertor::toNx(mRandPlace[place]), 3);
						mOgre->addTerrainItem(mRandPlace[place], ItemType(ITEM_NONE+type), ti->getItemId());
						mIsPlaced[place] = ti->getItemId();
					}
					
				}
				mTimeItemOld += 5000;
			}

			if (mTimeGet - mTimeGetOld - 10 >= 0)
			{
				if (!m_Paused)
				{
					++mTimeCount;
				}
				setTime(mTimeCount);
				mTimeGetOld += 10;
			}
			if (mTimeGet > 2000000000)
			{
				mMytime.reset();
				mTimeGet = 0;
				mTimeGetOld = 0;
				mTimeOilOld = 0;
			}
		}
		if (m_IsClosed)
		{
			return false;
		}
		return ExampleFrameListener::frameStarted(evt);
	}

	//	 MouseDragged
	virtual void mouseMoved (MouseEvent *e)
	{
		CEGUI::System::getSingleton().injectMouseMove(
			e->getRelX() * mGUIRenderer->getWidth(), 
			e->getRelY() * mGUIRenderer->getHeight());
		e->consume();
	}

	void mouseDragged (MouseEvent *e) 
	{ 
		mouseMoved(e);
	}

	void mousePressed (MouseEvent *e)
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(
			convertOgreButtonToCegui(e->getButtonID()));
		e->consume();
	}

	void mouseReleased (MouseEvent *e)
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(
			convertOgreButtonToCegui(e->getButtonID()));
		e->consume();
	}
	// MouseListener
	void mouseClicked(MouseEvent* e) { }
	void mouseEntered(MouseEvent* e) { }
	void mouseExited(MouseEvent* e) { }

	// KeyListener
	virtual void keyClicked(KeyEvent* e) { }
	virtual void keyPressed(KeyEvent* e) 
	{ 
		if(e->getKey() == KC_ESCAPE)
		{	
			if (mPlaying)
			{
				m_Paused = true;
				checkESC();
				e->consume();
				return;
			}
			else if (mIsMain)
			{
				m_IsClosed = true;
			}

			loadMain();		
			e->consume();
			return;
		}

		if (!mPlaying)
		{
			CEGUI::System::getSingleton().injectKeyDown(e->getKey());
			CEGUI::System::getSingleton().injectChar(e->getKeyChar());
			e->consume();
			return;
		}

		switch (e->getKey())
		{
		case KC_UP:
		case KC_W:
			m_Acceleration = 1;
			break;
		case KC_DOWN:
		case KC_S:
			m_Acceleration = -1;
			break;
		case KC_A:
		case KC_LEFT:
			m_Steering = -1;
			break;
		case KC_D:
		case KC_RIGHT:
			m_Steering = 1;
			break;
		case KC_R:
			mIsReset = true;
			break;
		case KC_TAB:
			if (mIsUserCamera)
			{
				mIsUserCamera = false;
				mOgre->wholeCamera(mCamera);
			}
			else
			{
				mIsUserCamera = true;
				mOgre->userCamera(mCamera, "tony");
			}
			break;
		case KC_SPACE:
			m_HandBrake = true;
			break;
		case KC_P:
			if (m_Paused)
			{
				m_Paused = false;
			}
			else
			{
				m_Paused = true;
			}
			break;
		case KC_1:
			mNumPressed = 1;
			break;
		case KC_2:
			mNumPressed = 2;
			break;
		case KC_3:
			mNumPressed = 3;
			break;
		case KC_4:
			mNumPressed = 4;
			break;
		case KC_5:
			mNumPressed = 5;
			break;
		case KC_6:
			mNumPressed = 6;
			break;
		}
	}
	virtual void keyReleased(KeyEvent* e) 
	{
		if (!mPlaying)
		{
			CEGUI::System::getSingleton().injectKeyUp(e->getKey());
			e->consume();
			return;
		}

		switch ( e->getKey() )
		{
		case KC_UP:
		case KC_W:
			m_Acceleration = 0;
			break;
		case KC_DOWN:
		case KC_S:
			m_Acceleration = 0;
			break;
		case KC_A:
		case KC_LEFT:
			m_Steering = 0;
			break;
		case KC_D:
		case KC_RIGHT:
			m_Steering = 0;
			break;
		case KC_SPACE:
			m_HandBrake = false;
			break;
		}

	}
};

class CuteCarApplication : public ExampleApplication
{
public:
	CuteCarApplication() : ExampleApplication()
	{
	}

	~CuteCarApplication()
	{
	}

	void chooseSceneManager(void)
	{
		//Use the terrain scene manager.
		mSceneMgr = mRoot->createSceneManager(ST_EXTERIOR_CLOSE);
	}

	void createCamera(void)
	{
		LogManager::getSingleton().createLog(LOG_FILE);

		// create camera, but leave at default position
		mCamera = mSceneMgr->createCamera("MyCam"); 

		mCamera->setPosition(Ogre::Vector3(70*SCALE_TERRAIN, 72*SCALE_TERRAIN, 90*SCALE_TERRAIN));
		mCamera->setOrientation(Ogre::Quaternion(0.981956, -0.189108, 0, 0));

		mCamera->setNearClipDistance(5);
	}

	void createScene(void)
	{
	}

	void createFrameListener(void)
	{
		// Create the FrameListener
		mFrameListener = new CuteCarFrameListener(mWindow, mCamera, mSceneMgr);
		mRoot->addFrameListener(mFrameListener);

		// Show the frame stats overlay
		mFrameListener->showDebugOverlay(false);
	}
};
