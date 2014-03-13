/********************************************************************
	created:	2006/05/13
	file base:	AudioClass
	file ext:	.h
	author:		Ethan
	version:	2.0
	purpose:	encapsulate the audio operation in 3D
*********************************************************************/
#pragma once
#ifndef _AUDIO_
#define _AUDIO_
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

class AudioClass;
class SoundClass;
class WaveClass;
/********************************************************************
	class name:	AudioClass
	desc:		Create the primary buffer
*********************************************************************/
class AudioClass{
public:
	AudioClass(HWND hwnd, bool is3D = true);
	~AudioClass();

public:
	HRESULT Initialize();
	void SetListenerParameters( float fDopplerFactor = 1, float fRolloffFactor = 1);
	void SetListenerPosition(D3DVECTOR vvector);
	void SetListenerPosition(float x, float y, float z);
	void SetListenerVelocity(D3DVECTOR vvector);
	void SetListenerVelocity(float x, float y, float z);
	void SetListenerOrientFront(D3DVECTOR vvector);
	void SetListenerOrientFront(float x, float y, float z);
	void SetListenerOrientTop(D3DVECTOR vvector);
	void SetListenerOrientTop(float x, float y, float z);
	void CommitDeferred(){ m_pDSListener->CommitDeferredSettings(); }
	DS3DLISTENER* GetListenerParams(){ 
		if (m_b3D)
			return &m_dsListenerParams; 
		else return NULL;
	}

	void SetDefer(bool bset){ m_bDeferSettings = bset; }
	bool IsDefer(){ return m_bDeferSettings; }
	void Set3D(bool b3D){ m_b3D = b3D; }
	bool Is3D(){ return m_b3D; }

	IDirectSound8* GetDirectSound(){ return m_pDS; }
	LPDIRECTSOUND3DLISTENER* GetListener(){ 
		if (m_b3D)
			return &m_pDSListener; 
		else return NULL;
	}

private:
	HRESULT InitializeMgr();
	HRESULT SetPrimaryBufferFormat( DWORD dwPrimaryChannels = 2, 
		                         DWORD dwPrimaryFreq = 22050,
								 DWORD dwPrimaryBitRate = 16 );
	HRESULT Get3DListenerInterface();

private:
	HWND					m_hWnd;				// the Handle of current wnd
	LPDIRECTSOUND8			m_pDS;				
	LPDIRECTSOUND3DLISTENER m_pDSListener;		// 3D listener object
	DS3DLISTENER            m_dsListenerParams;	// Listener properties
	LPDIRECTSOUNDBUFFER		m_pDSBPrimary;
	bool					m_bDeferSettings;	
	bool					m_b3D;
};

/********************************************************************
	class name:	SoundClass
	desc:		Manage DirectSound buffer
*********************************************************************/
class SoundClass{
public:
	SoundClass(AudioClass* pCrtDevice);
	SoundClass(AudioClass* pCrtDevice, LPSTR strWavName);
	virtual ~SoundClass();

public:
	HRESULT CreateBuffer(LPSTR strWaveName = "");
	HRESULT CreateBuffer(BYTE* pbData, DWORD dwBufferSize, LPWAVEFORMATEX pwfx);
	HRESULT Play( DWORD dwPriority = 0, LONG lVolume = 0, LONG lFrequency = -1, LONG lPan = 0 );
	HRESULT Stop();
	HRESULT Reset();
	bool    IsSoundPlaying();

	void	SetMinDistance(float fMinDistance);
	void	SetMaxDistance(float fMaxDistance);
	void	SetPosition(D3DVECTOR vvector);
	void	SetVelocity(D3DVECTOR vvector);
	void	SetConeOrientation(D3DVECTOR vvector);
	void	SetPosition(float x, float y, float z);
	void	SetVelocity(float x, float y, float z);
	void	SetConeOrientation(float x, float y, float z);
	void	SetInsideConeAngle(DWORD dwAngle);
	void	SetOutsideConeAngle(DWORD dwAngle);
	void	SetConeOutsideVolume(long lVolume);

	void	SetLoop(bool bloop){ m_bLooping = bloop; }
	bool	IsLoop(){ return m_bLooping; }
	
	LPDIRECTSOUNDBUFFER GetBuffer(){ return m_pDSBuffer; }
	DS3DBUFFER* GetBufferParams() {
		if (m_pAudioDevice->Is3D()) 
			return &m_dsBufferParams;
		else return NULL;
	}
	
protected:
	HRESULT Get3DBufferInterface();
	HRESULT RestoreBuffer( bool* pbWasRestored );
	HRESULT FillBufferWithSound( bool bRepeatWavIfBufferLarger );


protected:
	AudioClass*				m_pAudioDevice;
	LPDIRECTSOUNDBUFFER		m_pDSBuffer;	
	LPDIRECTSOUND3DBUFFER	m_pDS3DBuffer;
	DS3DBUFFER              m_dsBufferParams;
	DWORD					m_dwDSBufferSize;
	WaveClass*				m_pWaveFile;
	DWORD					m_dwCreationFlags;

	GUID					m_guid3DAlgorithm;
	LPSTR					m_strWavName;
	bool					m_bLooping;
};

/********************************************************************
class name:	WaveClass
desc:		Encapsulates reading or writing sound data to or from a wave file
*********************************************************************/
class WaveClass{
public:
	WaveClass();
	~WaveClass();

	HRESULT Open( LPSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags );
	HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags );
	HRESULT Close();

	HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
	HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

	DWORD   GetSize();
	HRESULT ResetFile();
	WAVEFORMATEX* GetFormat() { return m_pwfx; };

private:
	WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
	HMMIO         m_hmmio;       // MM I/O handle for the WAVE
	MMCKINFO      m_ck;          // Multimedia RIFF chunk
	MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
	DWORD         m_dwSize;      // The size of the wave file
	MMIOINFO      m_mmioinfoOut;
	DWORD         m_dwFlags;
	BOOL          m_bIsReadingFromMemory;
	BYTE*         m_pbData;
	BYTE*         m_pbDataCur;
	ULONG         m_ulDataSize;
	CHAR*         m_pResourceBuffer;

protected:
	HRESULT ReadMMIO();
	HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );
};
#endif