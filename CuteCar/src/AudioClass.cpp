/********************************************************************
created:	2006/05/13
file base:	AudioClass
file ext:	.cpp
author:		Ethan
version:	2.0
*********************************************************************/
#include "AudioClass.h"
//#include "stdafx.h"
/********************************************************************
class name:	AudioClass
purpose:	encapsulate the audio operation in 3D
*********************************************************************/
AudioClass::AudioClass(HWND hwnd, bool is3D){
    m_pDS = NULL;    
	m_pDSListener = NULL;
	m_hWnd = hwnd;
	m_pDSBPrimary = NULL;
	m_bDeferSettings = false;
	m_b3D = is3D;
}
AudioClass::~AudioClass(){
	SAFE_DELETE(m_pDS);
	SAFE_RELEASE(m_pDSListener);
	SAFE_RELEASE(m_pDSBPrimary);
}
HRESULT AudioClass::Initialize(){
	HRESULT hr ;
	hr = InitializeMgr();
	hr |= SetPrimaryBufferFormat();
	if (m_b3D) hr |= Get3DListenerInterface();
	if (FAILED(hr))
	{
		//MessageBox("Initialize Audio Device Error!");
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: AudioClass::InitializeMgr()
// Desc: Create DirectSound device
//-----------------------------------------------------------------------------
HRESULT AudioClass::InitializeMgr(){
	HRESULT hr;

	SAFE_RELEASE( m_pDS );

	// Create IDirectSound using the primary sound device
	if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) ){
		//MessageBox("Audio Sound Device Error!");
		return hr;
	}

	// Set DirectSound coop level
	if( FAILED( hr = m_pDS->SetCooperativeLevel( m_hWnd, DSSCL_PRIORITY ) ) ){
		//MessageBox("Set Cooperative Level Error!")
		return hr;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: AudioClass::SetPrimaryBufferFormat()
// Desc: Set primary buffer to a specified format
//-----------------------------------------------------------------------------
HRESULT AudioClass::SetPrimaryBufferFormat( DWORD dwPrimaryChannels,
											   DWORD dwPrimaryFreq,
											   DWORD dwPrimaryBitRate )
{
	HRESULT hr;

	if( m_pDS == NULL )
		return CO_E_NOTINITIALIZED;

	// Get the primary buffer
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	if (m_b3D) dsbd.dwFlags |= DSBCAPS_CTRL3D;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;

	if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &m_pDSBPrimary, NULL ) ) ){
		//MessageBox("Create Buffer Error!");
		return hr;
	}

	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) );
	wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM;
	wfx.nChannels       = (WORD) dwPrimaryChannels;
	wfx.nSamplesPerSec  = (DWORD) dwPrimaryFreq;
	wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate;
	wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	if( FAILED( hr = m_pDSBPrimary->SetFormat(&wfx) ) ){
		//MessageBox("Set Format Error!");
		return hr;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: AudioClass::Get3DBufferInterface()
// Desc: Get the Listener interface of DirectSound device
//-----------------------------------------------------------------------------
HRESULT AudioClass::Get3DListenerInterface()
{
	HRESULT             hr;

// 	if( m_pDSListener == NULL )
// 		return E_INVALIDARG;
	if( m_pDS == NULL )
		return CO_E_NOTINITIALIZED;

	m_pDSListener = NULL;
	
	if( FAILED( hr = m_pDSBPrimary->QueryInterface( IID_IDirectSound3DListener,(void**)&m_pDSListener ) ) )
	{
		SAFE_RELEASE( m_pDSBPrimary );
		//MessageBox("QueryInterFace Error!");
		return hr;
	}

	m_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
	m_pDSListener->GetAllParameters( &m_dsListenerParams );
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Set3DParameters()
// Desc: Set the 3D Listener parameters
//-----------------------------------------------------------------------------
// Every change to 3-D sound buffer and listener settings causes 
// DirectSound to remix, at the expense of CPU cycles. 
// To minimize the performance impact of changing 3-D settings, 
// use the DS3D_DEFERRED flag in the dwApply parameter of any of 
// the IDirectSound3DListener or IDirectSound3DBuffer methods that 
// change 3-D settings. Then call the IDirectSound3DListener::CommitDeferredSettings 
// method to execute all of the deferred commands at once.
void AudioClass::SetListenerParameters( float fDopplerFactor, float fRolloffFactor)
{
	if (m_b3D)
	{
		DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
		m_pDSListener->GetAllParameters(&m_dsListenerParams);
		if (fDopplerFactor > 0)
		{
			m_dsListenerParams.flDopplerFactor = fDopplerFactor;
		}
		if (fRolloffFactor > 0)
		{
			m_dsListenerParams.flRolloffFactor = fRolloffFactor;
		}
		if( m_pDSListener )
			m_pDSListener->SetAllParameters( &m_dsListenerParams, dwApplyFlag );
	}
	
}

void AudioClass::SetListenerPosition(D3DVECTOR vvector){
	if (m_b3D)
	{	
		DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
		m_pDSListener->GetAllParameters(&m_dsListenerParams);
		m_dsListenerParams.vPosition = vvector;
		if( m_pDSListener )
			m_pDSListener->SetAllParameters( &m_dsListenerParams, dwApplyFlag );
	}

}
void AudioClass::SetListenerPosition(float x, float y, float z){
	if (m_b3D)
	{
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetListenerPosition(temp);
	}
	
}
void AudioClass::SetListenerVelocity(D3DVECTOR vvector){
	if (m_b3D)
	{
		DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
		m_pDSListener->GetAllParameters(&m_dsListenerParams);
		m_dsListenerParams.vVelocity = vvector;
		if( m_pDSListener )
			m_pDSListener->SetAllParameters( &m_dsListenerParams, dwApplyFlag );
	}
	
}
void AudioClass::SetListenerVelocity(float x, float y, float z){
	if (m_b3D)
	{
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetListenerVelocity(temp);
	}
}
void AudioClass::SetListenerOrientFront(D3DVECTOR vvector){
	if (m_b3D)
	{	
		DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
		m_pDSListener->GetAllParameters(&m_dsListenerParams);
		m_dsListenerParams.vOrientFront = vvector;
		if( m_pDSListener )
			m_pDSListener->SetAllParameters( &m_dsListenerParams, dwApplyFlag );
	}

}
void AudioClass::SetListenerOrientFront(float x, float y, float z){
	if (m_b3D)
	{	
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetListenerOrientFront(temp);
	}

}
void AudioClass::SetListenerOrientTop(D3DVECTOR vvector){
	if (m_b3D)
	{
		DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;
		m_pDSListener->GetAllParameters(&m_dsListenerParams);
		m_dsListenerParams.vOrientTop = vvector;
		if( m_pDSListener )
			m_pDSListener->SetAllParameters( &m_dsListenerParams, dwApplyFlag );
	}
}
void AudioClass::SetListenerOrientTop(float x, float y, float z){
	if (m_b3D)
	{
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetListenerOrientTop(temp);
	}

}
/********************************************************************
class name:	SoundClass
purpose:	encapsulate the buffer operation in 3D
*********************************************************************/
SoundClass::SoundClass(AudioClass* pCrtDevice){
	m_pAudioDevice = pCrtDevice;
	m_guid3DAlgorithm = DS3DALG_HRTF_FULL;
	m_pDSBuffer = NULL;
	m_pWaveFile = NULL;
	m_pDS3DBuffer = NULL;
	m_bLooping = false;
}
SoundClass::SoundClass(AudioClass* pCrtDevice, LPSTR strWavName){
	m_pAudioDevice = pCrtDevice;
	m_strWavName = strWavName;
	m_guid3DAlgorithm = DS3DALG_HRTF_FULL;
	m_pDSBuffer = NULL;
	m_pWaveFile = NULL;
	m_pDS3DBuffer = NULL;
	m_bLooping = false;
}
SoundClass::~SoundClass(){
	this->Stop();
	this->Reset();

	SAFE_RELEASE(m_pDSBuffer);
	SAFE_DELETE(m_pWaveFile);
	SAFE_RELEASE(m_pDS3DBuffer);
}


//-----------------------------------------------------------------------------
// Name: CreateBuffer()
// Desc: Create the buffer for specific wavefile
//-----------------------------------------------------------------------------
HRESULT SoundClass::CreateBuffer(LPSTR strWaveName){
	if (strWaveName != ""){ m_strWavName = strWaveName; }
	HRESULT hr;
	HRESULT hrRet = S_OK;
	if( this ){
		this->Stop();
		this->Reset();
	}else{
		return E_INVALIDARG;
	}
	
	SAFE_RELEASE(m_pDSBuffer);
	SAFE_DELETE(m_pWaveFile);
	m_dwDSBufferSize = 0;

	if( m_pAudioDevice->GetDirectSound() == NULL )
		return CO_E_NOTINITIALIZED;
	if( m_strWavName == "")
		return E_INVALIDARG;

	m_pWaveFile = new WaveClass();
	if( m_pWaveFile == NULL )
	{
		hr = E_OUTOFMEMORY;
//		MessageBox("Out of Memory!");
		SAFE_RELEASE(m_pDSBuffer);
		return hr;
	}
	// Verify the file is small
	HANDLE hFile = CreateFileA( m_strWavName, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if( hFile != NULL )
	{
		// If you try to open a 100MB wav file, you could run out of system memory with this
		// sample cause it puts all of it into a large buffer.  If you need to do this, then 
		// see the "StreamData" sample to stream the data from the file into a sound buffer.
		DWORD dwFileSizeHigh = 0;
		DWORD dwFileSize = GetFileSize( hFile, &dwFileSizeHigh );
		CloseHandle( hFile );

		if( dwFileSizeHigh != 0 || dwFileSize > 1000000 )
		{
//			MessageBox("File too large. You should stream large files.");
			return S_FALSE;
		}
	}

	hr = m_pWaveFile->Open( m_strWavName, NULL, WAVEFILE_READ );
	if (FAILED(hr))
	{
//		MessageBox("Can not open wavefile");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
	}
	WAVEFORMATEX* pwfx = m_pWaveFile->GetFormat();
	if( pwfx == NULL )
	{
//		MessageBox("Invalid wave file format.");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
		return E_INVALIDARG;
	}

	if( pwfx->nChannels > 1 )
	{
		// Too many channels in wave.  Sound must be mono when using DSBCAPS_CTRL3D
//		MessageBox("Wave file must be mono for 3D control.");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
		return S_FALSE;
	}

	if( pwfx->wFormatTag != WAVE_FORMAT_PCM )
	{
		// Sound must be PCM when using DSBCAPS_CTRL3D
//		MessageBox("Wave file must be PCM for 3D control.");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
		return S_FALSE;
	}
	
	if( m_pWaveFile->GetSize() == 0 )
	{
		// Wave is blank, so don't create it.
		hr = E_FAIL;
//		MessageBox("Wave is blank!");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
		return hr;
	}

	// Make the DirectSound buffer the same size as the wav file
	m_dwDSBufferSize = m_pWaveFile->GetSize();

	// Create the direct sound buffer, and only request the flags needed
	// since each requires some overhead and limits if the buffer can
	// be hardware accelerated
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize          = sizeof(DSBUFFERDESC);
	dsbd.dwFlags         = DSBCAPS_GLOBALFOCUS|DSBCAPS_GETCURRENTPOSITION2;
	if (m_pAudioDevice->Is3D()) dsbd.dwFlags |= DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE;
	dsbd.dwBufferBytes   = m_dwDSBufferSize;
	if (m_pAudioDevice->Is3D()) dsbd.guid3DAlgorithm = m_guid3DAlgorithm;
	dsbd.lpwfxFormat     = m_pWaveFile->GetFormat();

	// DirectSound is only guarenteed to play PCM data.  Other
	// formats may or may not work depending the sound card driver.
	hr = m_pAudioDevice->GetDirectSound()->CreateSoundBuffer( &dsbd, &m_pDSBuffer, NULL );

	// Be sure to return this error code if it occurs so the
	// callers knows this happened.
	if( hr == DS_NO_VIRTUALIZATION )
		hrRet = DS_NO_VIRTUALIZATION;

	if( FAILED(hr) || hr == DS_NO_VIRTUALIZATION )
	{
		if( DS_NO_VIRTUALIZATION == hr )
		{
//			MessageBox( L"The 3D virtualization algorithm requested is not supported under this "
//				L"operating system.  It is available only on Windows 2000, Windows ME, and Windows 98 with WDM "
//				L"drivers and beyond.  Creating buffer with no virtualization.", 
//				L"DirectSound Sample");
		}

		// Unknown error, but not a critical failure, so just update the status
//		MessageBox("Could not create sound buffer.");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
		return hr; 
	}
	FillBufferWithSound(false);
	if (m_pAudioDevice->Is3D())
	{
		hr = Get3DBufferInterface();
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CreateBuffer()
// Desc: Create the buffer for specific wavefile from memory
//-----------------------------------------------------------------------------
HRESULT SoundClass::CreateBuffer(BYTE* pbData, DWORD dwBufferSize, LPWAVEFORMATEX pwfx){
	if (pbData == NULL){ return E_FAIL; }
	HRESULT hr;
	if( this ){
		this->Stop();
		this->Reset();
	}else{
		return E_INVALIDARG;
	}

	SAFE_RELEASE(m_pDSBuffer);
	SAFE_DELETE(m_pWaveFile);
	m_dwDSBufferSize = 0;

	if( m_pAudioDevice->GetDirectSound() == NULL )
		return CO_E_NOTINITIALIZED;
	if( m_strWavName == "")
		return E_INVALIDARG;

	m_pWaveFile = new WaveClass();
	if( m_pWaveFile == NULL )
	{
		hr = E_OUTOFMEMORY;
		//		MessageBox("Out of Memory!");
		SAFE_RELEASE(m_pDSBuffer);
		return hr;
	}
	hr = m_pWaveFile->OpenFromMemory( pbData, dwBufferSize, pwfx, WAVEFILE_READ);
	if (FAILED(hr))
	{
		//		MessageBox("Can not open wavefile");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
	}
	m_dwDSBufferSize = dwBufferSize;

	// Create the direct sound buffer, and only request the flags needed
	// since each requires some overhead and limits if the buffer can
	// be hardware accelerated
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize          = sizeof(DSBUFFERDESC);
	dsbd.dwFlags         = DSBCAPS_CTRLFREQUENCY|DSBCAPS_LOCDEFER;
	if (m_pAudioDevice->Is3D()) dsbd.dwFlags |= DSBCAPS_CTRL3D;
		dsbd.dwBufferBytes   = m_dwDSBufferSize;
	if (m_pAudioDevice->Is3D()) dsbd.guid3DAlgorithm = m_guid3DAlgorithm;
	dsbd.lpwfxFormat     = pwfx;

	// DirectSound is only guarenteed to play PCM data.  Other
	// formats may or may not work depending the sound card driver.
	hr = m_pAudioDevice->GetDirectSound()->CreateSoundBuffer( &dsbd, &m_pDSBuffer, NULL );

	// Be sure to return this error code if it occurs so the
	// callers knows this happened.
	
	if( FAILED(hr) || hr == DS_NO_VIRTUALIZATION )
	{
		if( DS_NO_VIRTUALIZATION == hr )
		{
			//			MessageBox( L"The 3D virtualization algorithm requested is not supported under this "
			//				L"operating system.  It is available only on Windows 2000, Windows ME, and Windows 98 with WDM "
			//				L"drivers and beyond.  Creating buffer with no virtualization.", 
			//				L"DirectSound Sample");
		}

		// Unknown error, but not a critical failure, so just update the status
		//		MessageBox("Could not create sound buffer.");
		SAFE_DELETE(m_pWaveFile);
		SAFE_RELEASE(m_pDSBuffer);
		return hr; 
	}
	FillBufferWithSound(false);
	if (m_pAudioDevice->Is3D())
	{
		hr = Get3DBufferInterface();
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

HRESULT SoundClass::Get3DBufferInterface(){
	HRESULT hr;
	// Get the 3D buffer from the secondary buffer
	hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&m_pDS3DBuffer);
	if( FAILED(hr) )
	{
		//		MessageBox("Could not get 3D buffer.");
		SAFE_RELEASE(m_pDS3DBuffer);
		return hr;
	}

	// Get the 3D buffer parameters
	m_dsBufferParams.dwSize = sizeof(DS3DBUFFER);
	m_pDS3DBuffer->GetAllParameters( &m_dsBufferParams );

	// Set new 3D buffer parameters
	m_dsBufferParams.dwMode = DS3DMODE_HEADRELATIVE;
	m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );

	return S_OK;
}
//-----------------------------------------------------------------------------
// Name: SoundClass::FillBufferWithSound()
// Desc: Fills a DirectSound buffer with a sound file
//-----------------------------------------------------------------------------
HRESULT SoundClass::FillBufferWithSound( bool bRepeatWavIfBufferLarger )
{
	HRESULT hr;
	void*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
	DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
	DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file

	if( m_pDSBuffer == NULL )
		return CO_E_NOTINITIALIZED;

	// Make sure we have focus, and we didn't just switch in from
	// an app which had a DirectSound device
	if( FAILED( hr = RestoreBuffer( NULL ) ) ){
//		MessageBox("Error Restore Buffer");
		return hr;
	}
	
	// Lock the buffer down
	if( FAILED( hr = m_pDSBuffer->Lock( 0, m_dwDSBufferSize,
										&pDSLockedBuffer, &dwDSLockedBufferSize,
										NULL, NULL, 0L ) ) ){
//		MessageBox("Error Lock");
		return hr;
	}

	// Reset the wave file to the beginning
	m_pWaveFile->ResetFile();

	if( FAILED( hr = m_pWaveFile->Read( (BYTE*) pDSLockedBuffer,
										dwDSLockedBufferSize,
										&dwWavDataRead ) ) ){
//		MessageBox("Error Read");
		return hr;
	}

	if( dwWavDataRead == 0 )
	{
		// Wav is blank, so just fill with silence
		FillMemory( (BYTE*) pDSLockedBuffer,
					dwDSLockedBufferSize,
					(BYTE)(m_pWaveFile->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
	}
	else if( dwWavDataRead < dwDSLockedBufferSize )
	{
		// If the wav file was smaller than the DirectSound buffer,
		// we need to fill the remainder of the buffer with data
		if( bRepeatWavIfBufferLarger )
		{
			// Reset the file and fill the buffer with wav data
			DWORD dwReadSoFar = dwWavDataRead;    // From previous call above.
			while( dwReadSoFar < dwDSLockedBufferSize )
			{
				// This will keep reading in until the buffer is full
				// for very short files
				if( FAILED( hr = m_pWaveFile->ResetFile() ) ){
//					MessageBox("Error Reset");
					return hr;
				}

				hr = m_pWaveFile->Read( (BYTE*)pDSLockedBuffer + dwReadSoFar,
					dwDSLockedBufferSize - dwReadSoFar,
					&dwWavDataRead );
				if( FAILED(hr) ){
//					MessageBox("Error Read");
					return hr;
				}

				dwReadSoFar += dwWavDataRead;
			}
		}
		else
		{
			// Don't repeat the wav file, just fill in silence
			FillMemory( (BYTE*) pDSLockedBuffer + dwWavDataRead,
				dwDSLockedBufferSize - dwWavDataRead,
				(BYTE)(m_pWaveFile->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
		}
	}

	// Unlock the buffer, we don't need it anymore.
	m_pDSBuffer->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: SoundClass::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT SoundClass::RestoreBuffer( bool* pbWasRestored )
{
	HRESULT hr;

	if( m_pDSBuffer == NULL )
		return CO_E_NOTINITIALIZED;
	if( pbWasRestored )
		*pbWasRestored = false;

	DWORD dwStatus;
	if( FAILED( hr = m_pDSBuffer->GetStatus( &dwStatus ) ) ){
//		MessageBox("GetStatus");	
		return hr;
	}

	if( dwStatus & DSBSTATUS_BUFFERLOST )
	{
		// Since the app could have just been activated, then
		// DirectSound may not be giving us control yet, so
		// the restoring the buffer may fail.
		// If it does, sleep until DirectSound gives us control.
		while( ( hr = m_pDSBuffer->Restore() ) == DSERR_BUFFERLOST ){
			Sleep( 10 );
		}
		

		if( pbWasRestored != NULL )
			*pbWasRestored = true;

		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

//-----------------------------------------------------------------------------
// Name: SoundClass::Play()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT SoundClass::Play( DWORD dwPriority,LONG lVolume, LONG lFrequency, LONG lPan )
{
	HRESULT hr;
	bool    bRestored;

	if( m_pDSBuffer == NULL )
		return CO_E_NOTINITIALIZED;

	// Restore the buffer if it was lost
	//if( FAILED( hr = RestoreBuffer( &bRestored ) ) ){
//		MessageBox("Restore Buffer");
	//	return hr;
	//}
	//if (bRestored)
	//{
		// The buffer was restored, so we need to fill it with new data
	//	if( FAILED( hr = FillBufferWithSound( false ) ) ){
//			MessageBox("FillBufferWithSound");
	//		return hr;
	//	}
	//}
	/*if (m_pAudioDevice->Is3D())
	{
		if( m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY )
		{
			DWORD dwBaseFrequency;
			m_pDSBuffer->GetFrequency( &dwBaseFrequency );
			m_pDSBuffer->SetFrequency( dwBaseFrequency + lFrequency );
		}

	}
	else{
		if( m_dwCreationFlags & DSBCAPS_CTRLVOLUME )
		{
			m_pDSBuffer->SetVolume( lVolume );
		}

		if( lFrequency != -1 &&
			(m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY) )
		{
			m_pDSBuffer->SetFrequency( lFrequency );
		}

		if( m_dwCreationFlags & DSBCAPS_CTRLPAN )
		{
			m_pDSBuffer->SetPan( lPan );
		}
	}*/
	
	DWORD dwFlags = (m_bLooping)?DSBPLAY_LOOPING:NULL;
	return m_pDSBuffer->Play( 0, dwPriority, dwFlags );
}

//-----------------------------------------------------------------------------
// Name: SoundClass::Stop()
// Desc: Stops the sound from playing
//-----------------------------------------------------------------------------
HRESULT SoundClass::Stop()
{
	if( m_pDSBuffer == NULL )
		return CO_E_NOTINITIALIZED;

	HRESULT hr = 0;

	hr = m_pDSBuffer->Stop();
	return hr;
}


//-----------------------------------------------------------------------------
// Name: SoundClass::Reset()
// Desc: Reset all of the sound buffers
//-----------------------------------------------------------------------------
HRESULT SoundClass::Reset()
{
	if( m_pDSBuffer == NULL )
		return CO_E_NOTINITIALIZED;

	HRESULT hr = 0;
	hr = m_pDSBuffer->SetCurrentPosition( 0 );

	return hr;
}

//-----------------------------------------------------------------------------
// Name: SoundClass::IsSoundPlaying()
// Desc: Checks to see if a buffer is playing and returns TRUE if it is.
//-----------------------------------------------------------------------------
bool SoundClass::IsSoundPlaying()
{
	bool bIsPlaying = false;

	if( m_pDSBuffer == NULL )
		return bIsPlaying;

	DWORD dwStatus = 0;
	m_pDSBuffer->GetStatus( &dwStatus );
	bIsPlaying = ( (dwStatus & DSBSTATUS_PLAYING) != 0 );
		
	return bIsPlaying;
}
void SoundClass::SetMinDistance(float fMinDistance){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.flMinDistance = fMinDistance;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}

}
void SoundClass::SetMaxDistance(float fMaxDistance){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.flMaxDistance = fMaxDistance;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}

}
void SoundClass::SetPosition(D3DVECTOR vvector){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.vPosition = vvector;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}

}
void SoundClass::SetPosition(float x, float y, float z){
	if (m_pAudioDevice->Is3D())
	{	
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetPosition(temp);
	}
}
void SoundClass::SetVelocity(D3DVECTOR vvector){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.vVelocity = vvector;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}
}
void SoundClass::SetVelocity(float x, float y, float z){
	if (m_pAudioDevice->Is3D())
	{
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetVelocity(temp);
	}
}
void SoundClass::SetConeOrientation(D3DVECTOR vvector){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.vConeOrientation = vvector;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}
}

void SoundClass::SetConeOrientation(float x, float y, float z){
	if (m_pAudioDevice->Is3D())
	{
		D3DVECTOR temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		SetConeOrientation(temp);
	}
}
void SoundClass::SetInsideConeAngle(DWORD dwAngle){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.dwInsideConeAngle = dwAngle;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}
}
void SoundClass::SetOutsideConeAngle(DWORD dwAngle){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.dwOutsideConeAngle = dwAngle;
		if( m_pDS3DBuffer)
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}
}
void SoundClass::SetConeOutsideVolume(long lVolume){
	if (m_pAudioDevice->Is3D())
	{
		m_pDS3DBuffer->GetAllParameters(&m_dsBufferParams);
		m_dsBufferParams.lConeOutsideVolume = lVolume;
		if( m_pDS3DBuffer )
			m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
	}
}
/********************************************************************
class name:	WaveClass
desc:		Encapsulates reading or writing sound data to or from a wave file
*********************************************************************/
//-----------------------------------------------------------------------------
// Name: WaveClass::WaveClass()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.
//       Then call Read() as needed.  Calling the destructor or Close()
//       will close the file.
//-----------------------------------------------------------------------------
WaveClass::WaveClass()
{
	m_pwfx    = NULL;
	m_hmmio   = NULL;
	m_pResourceBuffer = NULL;
	m_dwSize  = 0;
	m_bIsReadingFromMemory = FALSE;
}

//-----------------------------------------------------------------------------
// Name: WaveClass::~WaveClass()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
WaveClass::~WaveClass()
{
	Close();

	if( !m_bIsReadingFromMemory )
		SAFE_DELETE_ARRAY( m_pwfx );
}

//-----------------------------------------------------------------------------
// Name: WaveClass::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT WaveClass::Open( LPSTR strWaveName, WAVEFORMATEX* pwfx, DWORD dwFlags )
{
	HRESULT hr;

	m_dwFlags = dwFlags;
	m_bIsReadingFromMemory = FALSE;

	if( m_dwFlags == WAVEFILE_READ )
	{
		if( strWaveName == NULL )
			return E_INVALIDARG;
		SAFE_DELETE_ARRAY( m_pwfx );

		m_hmmio = mmioOpenA( strWaveName, NULL, MMIO_ALLOCBUF | MMIO_READ );

		if( NULL == m_hmmio )
		{
			HRSRC   hResInfo;
			HGLOBAL hResData;
			DWORD   dwSize;
			VOID*   pvRes;

			// Loading it as a file failed, so try it as a resource
			if( NULL == ( hResInfo = FindResourceA( NULL, strWaveName, "WAVE" ) ) )
			{
				if( NULL == ( hResInfo = FindResourceA( NULL, strWaveName, "WAV" ) ) )
					return E_FAIL;
			}

			if( NULL == ( hResData = LoadResource( GetModuleHandle(NULL), hResInfo ) ) )
				return E_FAIL;

			if( 0 == ( dwSize = SizeofResource( GetModuleHandle(NULL), hResInfo ) ) )
				return E_FAIL;

			if( NULL == ( pvRes = LockResource( hResData ) ) )
				return E_FAIL;

			m_pResourceBuffer = new CHAR[ dwSize ];
			if( m_pResourceBuffer == NULL )
				return E_OUTOFMEMORY;
			memcpy( m_pResourceBuffer, pvRes, dwSize );

			MMIOINFO mmioInfo;
			ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
			mmioInfo.fccIOProc = FOURCC_MEM;
			mmioInfo.cchBuffer = dwSize;
			mmioInfo.pchBuffer = (CHAR*) m_pResourceBuffer;

			m_hmmio = mmioOpen( NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ );
		}

		if( FAILED( hr = ReadMMIO() ) )
		{
			// ReadMMIO will fail if its an not a wave file
			mmioClose( m_hmmio, 0 );
			return hr;
		}

		if( FAILED( hr = ResetFile() ) )
			return hr;

		// After the reset, the size of the wav file is m_ck.cksize so store it now
		m_dwSize = m_ck.cksize;
	}
	else
	{
		m_hmmio = mmioOpenA( strWaveName, NULL, MMIO_ALLOCBUF|MMIO_READWRITE|MMIO_CREATE );
		if( NULL == m_hmmio )
			return E_FAIL;

		if( FAILED( hr = WriteMMIO( pwfx ) ) )
		{
			mmioClose( m_hmmio, 0 );
			return hr;
		}

		if( FAILED( hr = ResetFile() ) )
			return hr;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// Name: WaveClass::OpenFromMemory()
// Desc: copy data to WaveClass member variable from memory
//-----------------------------------------------------------------------------
HRESULT WaveClass::OpenFromMemory( BYTE* pbData, ULONG ulDataSize,
								  WAVEFORMATEX* pwfx, DWORD dwFlags )
{
	m_pwfx       = pwfx;
	m_ulDataSize = ulDataSize;
	m_pbData     = pbData;
	m_pbDataCur  = m_pbData;
	m_bIsReadingFromMemory = TRUE;

	if( dwFlags != WAVEFILE_READ )
		return E_NOTIMPL;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: WaveClass::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx.
//-----------------------------------------------------------------------------
HRESULT WaveClass::ReadMMIO()
{
	MMCKINFO        ckIn;           // chunk info. for general use.
	PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.

	m_pwfx = NULL;

	if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) )
		return E_FAIL;

	// Check to make sure this is a valid wave file
	if( (m_ckRiff.ckid != FOURCC_RIFF) ||
		(m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
		return E_FAIL;

	// Search the input file for for the 'fmt ' chunk.
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
		return E_FAIL;

	// Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
	// if there are extra parameters at the end, we'll ignore them
	if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
		return E_FAIL;

	// Read the 'fmt ' chunk into <pcmWaveFormat>.
	if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat,
		sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
		return E_FAIL;

	// Allocate the waveformatex, but if its not pcm format, read the next
	// word, and thats how many extra bytes to allocate.
	if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
	{
		m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) ];
		if( NULL == m_pwfx )
			return E_FAIL;

		// Copy the bytes from the pcm structure to the waveformatex structure
		memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
		m_pwfx->cbSize = 0;
	}
	else
	{
		// Read in length of extra bytes.
		WORD cbExtraBytes = 0L;
		if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
			return E_FAIL;

		m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
		if( NULL == m_pwfx )
			return E_FAIL;

		// Copy the bytes from the pcm structure to the waveformatex structure
		memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
		m_pwfx->cbSize = cbExtraBytes;

		// Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
		if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),
			cbExtraBytes ) != cbExtraBytes )
		{
			SAFE_DELETE( m_pwfx );
			return E_FAIL;
		}
	}

	// Ascend the input file out of the 'fmt ' chunk.
	if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
	{
		SAFE_DELETE( m_pwfx );
		return E_FAIL;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: WaveClass::GetSize()
// Desc: Retuns the size of the read access wave file
//-----------------------------------------------------------------------------
DWORD WaveClass::GetSize()
{
	return m_dwSize;
}

//-----------------------------------------------------------------------------
// Name: WaveClass::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the
//       beginning of the file again
//-----------------------------------------------------------------------------
HRESULT WaveClass::ResetFile()
{
	if( m_bIsReadingFromMemory )
	{
		m_pbDataCur = m_pbData;
	}
	else
	{
		if( m_hmmio == NULL )
			return CO_E_NOTINITIALIZED;

		if( m_dwFlags == WAVEFILE_READ )
		{
			// Seek to the data
			if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
				SEEK_SET ) )
				return E_FAIL;

			// Search the input file for the 'data' chunk.
			m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
			if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
				return E_FAIL;
		}
		else
		{
			// Create the 'data' chunk that holds the waveform samples.
			m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
			m_ck.cksize = 0;

			if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
				return E_FAIL;

			if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
				return E_FAIL;
		}
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: WaveClass::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So
//       subsequent calls will be continue where the last left off unless
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT WaveClass::Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead )
{
	if( m_bIsReadingFromMemory )
	{
		if( m_pbDataCur == NULL )
			return CO_E_NOTINITIALIZED;
		if( pdwSizeRead != NULL )
			*pdwSizeRead = 0;

		if( (BYTE*)(m_pbDataCur + dwSizeToRead) >
			(BYTE*)(m_pbData + m_ulDataSize) )
		{
			dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
		}

		CopyMemory( pBuffer, m_pbDataCur, dwSizeToRead );

		if( pdwSizeRead != NULL )
			*pdwSizeRead = dwSizeToRead;

		return S_OK;
	}
	else
	{
		MMIOINFO mmioinfoIn; // current status of m_hmmio

		if( m_hmmio == NULL )
			return CO_E_NOTINITIALIZED;
		if( pBuffer == NULL || pdwSizeRead == NULL )
			return E_INVALIDARG;

		if( pdwSizeRead != NULL )
			*pdwSizeRead = 0;

		if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
			return E_FAIL;

		UINT cbDataIn = dwSizeToRead;
		if( cbDataIn > m_ck.cksize )
			cbDataIn = m_ck.cksize;

		m_ck.cksize -= cbDataIn;

		for( DWORD cT = 0; cT < cbDataIn; cT++ )
		{
			// Copy the bytes from the io to the buffer.
			if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
			{
				if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
					return E_FAIL;

				if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
					return E_FAIL;
			}

			// Actual copy.
			*((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
			mmioinfoIn.pchNext++;
		}

		if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
			return E_FAIL;

		if( pdwSizeRead != NULL )
			*pdwSizeRead = cbDataIn;

		return S_OK;
	}
}


//-----------------------------------------------------------------------------
// Name: WaveClass::Close()
// Desc: Closes the wave file
//-----------------------------------------------------------------------------
HRESULT WaveClass::Close()
{
	if( m_dwFlags == WAVEFILE_READ )
	{
		mmioClose( m_hmmio, 0 );
		m_hmmio = NULL;
		SAFE_DELETE_ARRAY( m_pResourceBuffer );
	}
	else
	{
		m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

		if( m_hmmio == NULL )
			return CO_E_NOTINITIALIZED;

		if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
			return E_FAIL;

		// Ascend the output file out of the 'data' chunk -- this will cause
		// the chunk size of the 'data' chunk to be written.
		if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
			return E_FAIL;

		// Do this here instead...
		if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
			return E_FAIL;

		mmioSeek( m_hmmio, 0, SEEK_SET );

		if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
			return E_FAIL;

		m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

		if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
		{
			DWORD dwSamples = 0;
			mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(DWORD) );
			mmioAscend( m_hmmio, &m_ck, 0 );
		}

		// Ascend the output file out of the 'RIFF' chunk -- this will cause
		// the chunk size of the 'RIFF' chunk to be written.
		if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
			return E_FAIL;

		mmioClose( m_hmmio, 0 );
		m_hmmio = NULL;
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: WaveClass::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.
//-----------------------------------------------------------------------------
HRESULT WaveClass::WriteMMIO( WAVEFORMATEX *pwfxDest )
{
	DWORD    dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
	MMCKINFO ckOut1;

	dwFactChunk = (DWORD)-1;

	// Create the output file RIFF chunk of form type 'WAVE'.
	m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	m_ckRiff.cksize = 0;

	if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) )
		return E_FAIL;

	// We are now descended into the 'RIFF' chunk we just created.
	// Now create the 'fmt ' chunk. Since we know the size of this chunk,
	// specify it in the MMCKINFO structure so MMIO doesn't have to seek
	// back and set the chunk size after ascending from the chunk.
	m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
	m_ck.cksize = sizeof(PCMWAVEFORMAT);

	if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
		return E_FAIL;

	// Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type.
	if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
	{
		if( mmioWrite( m_hmmio, (HPSTR) pwfxDest,
			sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
			return E_FAIL;
	}
	else
	{
		// Write the variable length size.
		if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest,
			sizeof(*pwfxDest) + pwfxDest->cbSize ) !=
			( sizeof(*pwfxDest) + pwfxDest->cbSize ) )
			return E_FAIL;
	}

	// Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
	if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
		return E_FAIL;

	// Now create the fact chunk, not required for PCM but nice to have.  This is filled
	// in when the close routine is called.
	ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
	ckOut1.cksize = 0;

	if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) )
		return E_FAIL;

	if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) !=
		sizeof(dwFactChunk) )
		return E_FAIL;

	// Now ascend out of the fact chunk...
	if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) )
		return E_FAIL;

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: WaveClass::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
HRESULT WaveClass::Write( UINT nSizeToWrite, BYTE* pbSrcData, UINT* pnSizeWrote )
{
	UINT cT;

	if( m_bIsReadingFromMemory )
		return E_NOTIMPL;
	if( m_hmmio == NULL )
		return CO_E_NOTINITIALIZED;
	if( pnSizeWrote == NULL || pbSrcData == NULL )
		return E_INVALIDARG;

	*pnSizeWrote = 0;

	for( cT = 0; cT < nSizeToWrite; cT++ )
	{
		if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite )
		{
			m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
			if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) )
				return E_FAIL;
		}

		*((BYTE*)m_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData+cT);
		(BYTE*)m_mmioinfoOut.pchNext++;

		(*pnSizeWrote)++;
	}

	return S_OK;
}