#ifndef MF_MEDIA_PLAYER_H_
#define MF_MEDIA_PLAYER_H_

#include <Windows.h>
#include <mfmediaengine.h>

class CMfMediaPlayerNotify : public IMFMediaEngineNotify
{
public:
	CMfMediaPlayerNotify(void* pMediaPlayer);
	~CMfMediaPlayerNotify();

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == __uuidof(IMFMediaEngineNotify))
		{
			*ppv = static_cast<IMFMediaEngineNotify*>(this);
		}
		else
		{
			*ppv = nullptr;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
	STDMETHODIMP EventNotify(DWORD Event, DWORD_PTR param1, DWORD param2)
	{
		if (Event == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE)
		{
			::SetEvent(reinterpret_cast<HANDLE>(param1));
		}
		else
		{
			onMediaEngineEvent(Event, param1, param2);
		}
		return S_OK;
	}
	STDMETHODIMP_(ULONG) AddRef() { return ::InterlockedIncrement(&m_lRef); }
	STDMETHODIMP_(ULONG) Release()
	{
		LONG lRef = ::InterlockedDecrement(&m_lRef);
		if (!lRef)delete this;
		return lRef;
	}

private:
	LONG m_lRef = 0;
	void* m_pMediaPlayer = nullptr;

	void onMediaEngineEvent(DWORD Event, DWORD_PTR param1, DWORD param2);
};

class CMfMediaPlayer
{
public:
	CMfMediaPlayer();
	virtual ~CMfMediaPlayer();

	bool play(const wchar_t* filePath);

	bool setLoop(bool looped);
	bool isLooped();

	bool setMute(bool muted);
	bool isMuted();

	bool setPause(bool paused);
	bool isPaused();

	bool isEnded();

	/// @brief Step one frame forward/backward
	bool frameStep(bool forward);

	bool setCurrentVolume(double dbVolume);
	double getCurrentVolume();

	bool setCurrentRate(double dbRate);
	double getCurrentRate();

	long long getCurrentTimeInMilliSeconds();

	/// @brief Set window and message code to receive event notification.
	virtual bool setPlaybackWindow(HWND hWnd, UINT uMsg = 0);
	bool getVideoSize(DWORD* dwWidth, DWORD* dwHeight);
	void setDisplayArea(const RECT absoluteRect);
	virtual bool resizeBuffer();

	HWND getRetHwnd()const { return m_hRetWnd; }
	UINT getRetMsg() const { return m_uRetMsg; }
protected:
	HWND m_hRetWnd = nullptr;
	UINT m_uRetMsg = 0;

	HRESULT m_hrComInit = E_FAIL;
	HRESULT m_hrMfStart = E_FAIL;

	CMfMediaPlayerNotify* m_pMfNotify = nullptr;
	IMFMediaEngineEx* m_pMfEngineEx = nullptr;
	IMFAttributes* m_pMfAttributes = nullptr;

	MFVideoNormalizedRect m_normalisedRect{};

	void workOutNormalisedRect(const RECT absoluteRect, MFVideoNormalizedRect* normalisedRect);
};

#endif // !MF_MEDIA_PLAYER_H_
