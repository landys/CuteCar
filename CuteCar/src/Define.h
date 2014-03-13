#pragma once

#define VEHICLE_MAX_ITEMS 6
#define SCALE_TERRAIN 2.8
#define OGRE_TERRAIN_FILE "MyTerrain.mesh"
#define OGRE_SCRATCH_LINE_FILE "lubiao.mesh"
#define OGRE_VEHICLE_FILE "cheti.mesh"
#define OGRE_WHEEL_FILE "lunzi.mesh"
#define LOG_FILE "CuteCar.log"
#define NX_TERRAIN_FILE "..\\resources\\map\\MyTerrainOnly.terr"
#define INITLOG Ogre::Log* pLog = Ogre::LogManager::getSingleton().getLog(LOG_FILE)
#define PRINTLOG(x) pLog->logMessage(#x "=" + Ogre::StringConverter::toString(x))
#define DEFAULT_GRAVITY (NxVec3(0.0f, -10.0f*5, 0.0f))
#define SCORE_FILE "..\\resources\\grade"

// 道具文件名
#define OGRE_ITEM_SLIP_FILE "brush.mesh"
#define OGRE_ITEM_BIGOIL_FILE "pliers.mesh"
#define OGRE_ITEM_MIDOIL_FILE "pliers.mesh"
#define OGRE_ITEM_SMALLOIL_FILE "pliers.mesh"
#define OGRE_ITEM_SPEEDUP_FILE "hummer.mesh"
#define OGRE_ITEM_BLOCK_FILE "nail.mesh"
#define OGRE_ITEM_ILL_FILE "ax.mesh"

// 音乐文件名
#define SOUND_TRAVEL_FILE "..\\resources\\CarTravel.wav"
#define SOUND_GET_ITEM_FILE "..\\resources\\GetItem.wav"
#define SOUND_USE_ITEM_FILE "..\\resources\\UseItem.wav"
#define SOUND_BEGIN_FILE "..\\resources\\begin.wav"
#define SOUND_END_FILE "..\\resources\\end.wav"


enum ItemType{ ITEM_NONE, ITEM_TYPE_SLIP, ITEM_TYPE_BIGOIL, ITEM_TYPE_MIDOIL, ITEM_TYPE_SMALLOIL, 
	ITEM_TYPE_SPEEDUP, ITEM_TYPE_BLOCK, ITEM_TYPE_ILL, ITEM_SCRATCH_LINE_IN, ITEM_SCRATCH_LINE_OUT };
