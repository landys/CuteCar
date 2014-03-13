/*
*   LoadLibs.h
*   automatically includes libs in the project
*/

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "OgreMain_d.lib")
#pragma comment(lib, "OgreGUIRenderer_d.lib")
#pragma comment(lib, "CEGUIBase_d.lib")
#else
#pragma comment(lib, "OgreMain.lib")
#pragma comment(lib, "OgreGUIRenderer.lib")
#pragma comment(lib, "CEGUIBase.lib")
#endif

#pragma comment(lib, "PhysXLoader.lib")
#pragma comment(lib, "NxCooking.lib")
//#pragma comment(lib, "NxCharacter.lib")
#pragma comment(lib, "SampleCommonCodeDEBUG.lib")

// DirectSound
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3dx9d.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxerr.lib")

// RakNet
//#pragma comment(lib, "RakNetLibStatic.lib")
//#pragma comment(lib, "WSock32.lib")
