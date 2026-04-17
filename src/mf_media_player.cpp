

#include <atlbase.h>
#include <mfapi.h>

#include "mf_media_player.h"

#pragma comment (lib,"Mfplat.lib")

CMfMediaPlayerNotify::CMfMediaPlayerNotify(void* pMediaPlayer)
	:m_pMediaPlayer(pMediaPlayer)
{

}

CMfMediaPlayerNotify::~CMfMediaPlayerNotify()
{

}

void CMfMediaPlayerNotify::onMediaEngineEvent(DWORD Event, DWORD_PTR param1, DWORD param2)
{
	CMfMediaPlayer* pMediaPlayer = static_cast<CMfMediaPlayer*>(m_pMediaPlayer);
	if (pMediaPlayer != nullptr)
	{
		const HWND hWnd = pMediaPlayer->getRetHwnd();
		const UINT uMsg = pMediaPlayer->getRetMsg();
		if (hWnd != nullptr && uMsg != 0)
		{
			::PostMessage(hWnd, uMsg, param1, Event);
		}
	}
}

CMfMediaPlayer::CMfMediaPlayer()
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	m_hrMfStart = ::MFStartup(MF_VERSION);
	if (FAILED(m_hrMfStart))return;

	CComPtr<IMFMediaEngineClassFactory> pMfFactory;
	CComPtr<IMFMediaEngine> pMfMediaEngine;

	HRESULT hr = ::CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pMfFactory));
	if (FAILED(hr))return;

	hr = ::MFCreateAttributes(&m_pMfAttributes, 1);
	if (FAILED(hr))return;

	m_pMfNotify = new CMfMediaPlayerNotify(this);
	hr = m_pMfAttributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, reinterpret_cast<IUnknown*>(m_pMfNotify));
	if (FAILED(hr))goto failed;

	hr = pMfFactory->CreateInstance(MF_MEDIA_ENGINE_REAL_TIME_MODE, m_pMfAttributes, &pMfMediaEngine);
	if (FAILED(hr))goto failed;

	hr = pMfMediaEngine->QueryInterface(__uuidof(IMFMediaEngineEx), (void**)&m_pMfEngineEx);
	if (FAILED(hr))goto failed;

	m_pMfEngineEx->SetVolume(0.5);
	m_pMfEngineEx->SetPreload(MF_MEDIA_ENGINE_PRELOAD_METADATA);

	return;
failed:
	if (m_pMfNotify != nullptr)
	{
		m_pMfNotify->Release();
		m_pMfNotify = nullptr;
	}
	if (m_pMfAttributes != nullptr)
	{
		m_pMfAttributes->Release();
		m_pMfAttributes = nullptr;
	}
}

CMfMediaPlayer::~CMfMediaPlayer()
{
	if (m_pMfAttributes != nullptr)
	{
		m_pMfAttributes->Release();
		m_pMfAttributes = nullptr;
	}
	if (m_pMfEngineEx != nullptr)
	{
		m_pMfEngineEx->Shutdown();
		m_pMfEngineEx = nullptr;
	}

	if (SUCCEEDED(m_hrMfStart))
	{
		::MFShutdown();
		m_hrMfStart = E_FAIL;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
		m_hrComInit = E_FAIL;
	}
}

bool CMfMediaPlayer::play(const wchar_t* filePath)
{
	if (m_pMfEngineEx != nullptr)
	{
		if (filePath != nullptr)
		{
			HRESULT hr = m_pMfEngineEx->SetSource(const_cast<BSTR>(filePath));
		}
		return SUCCEEDED(m_pMfEngineEx->Play());
	}
	return false;
}

bool CMfMediaPlayer::setLoop(bool looped)
{
	if (m_pMfEngineEx != nullptr)
	{
		HRESULT hr = m_pMfEngineEx->SetLoop(looped ? TRUE : FALSE);

		return SUCCEEDED(hr);
	}

	return false;
}

bool CMfMediaPlayer::isLooped()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetLoop() == TRUE;
	}

	return false;
}

bool CMfMediaPlayer::setMute(bool muted)
{
	if (m_pMfEngineEx != nullptr)
	{
		HRESULT hr = m_pMfEngineEx->SetMuted(muted ? TRUE : FALSE);

		return SUCCEEDED(hr);
	}

	return false;
}

bool CMfMediaPlayer::isMuted()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetMuted() == TRUE;
	}

	return false;
}

bool CMfMediaPlayer::setPause(bool paused)
{
	if (m_pMfEngineEx != nullptr)
	{
		HRESULT hr = paused ? m_pMfEngineEx->Pause() : m_pMfEngineEx->Play();
		if (SUCCEEDED(hr))
		{
			if (paused)
			{
				frameStep(true);
			}

			return true;
		}
	}

	return false;
}

bool CMfMediaPlayer::isPaused()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->IsPaused() == TRUE;
	}

	return false;
}

bool CMfMediaPlayer::isEnded()
{
	if (m_pMfEngineEx != nullptr)
	{
		BOOL iRet = m_pMfEngineEx->HasAudio();
		iRet |= m_pMfEngineEx->HasVideo();
		if (!iRet)
		{
			return true;
		}
		else
		{
			return m_pMfEngineEx->IsEnded() == TRUE;
		}
	}

	return true;
}

bool CMfMediaPlayer::frameStep(bool forward)
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->FrameStep(forward ? TRUE : FALSE) == TRUE;
	}

	return false;
}

bool CMfMediaPlayer::setCurrentVolume(double dbVolume)
{
	if (m_pMfEngineEx != nullptr)
	{
		return SUCCEEDED(m_pMfEngineEx->SetVolume(dbVolume));
	}
	return false;
}

double CMfMediaPlayer::getCurrentVolume()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetVolume();
	}
	return 100.0;
}

bool CMfMediaPlayer::setCurrentRate(double dbRate)
{
	if (m_pMfEngineEx != nullptr)
	{
		if (dbRate != m_pMfEngineEx->GetDefaultPlaybackRate())
		{
			m_pMfEngineEx->SetPlaybackRate(dbRate);
		}
		return SUCCEEDED(m_pMfEngineEx->SetDefaultPlaybackRate(dbRate));
	}
	return false;
}

double CMfMediaPlayer::getCurrentRate()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetPlaybackRate();
	}
	return 1.0;
}

long long CMfMediaPlayer::getCurrentTimeInMilliSeconds()
{
	if (m_pMfEngineEx != nullptr)
	{
		double dbTime = m_pMfEngineEx->GetCurrentTime();
		return static_cast<long long>(::round(dbTime * 1000));
	}

	return 0;
}

bool CMfMediaPlayer::setPlaybackWindow(HWND hWnd, UINT uMsg)
{
	m_hRetWnd = hWnd;
	m_uRetMsg = uMsg;

	HRESULT hr = m_pMfAttributes->SetUINT64(MF_MEDIA_ENGINE_PLAYBACK_HWND, reinterpret_cast<UINT64>(m_hRetWnd));
	return SUCCEEDED(hr);
}
/*動画縦横幅取得*/
bool CMfMediaPlayer::getVideoSize(DWORD* dwWidth, DWORD* dwHeight)
{
	if (dwWidth != nullptr && dwHeight != nullptr)
	{
		if (m_pMfEngineEx != nullptr)
		{
			BOOL iRet = m_pMfEngineEx->HasVideo();
			if (iRet)
			{
				HRESULT hr = m_pMfEngineEx->GetNativeVideoSize(dwWidth, dwHeight);
				return SUCCEEDED(hr);
			}
		}
	}

	return false;
}
/*表示範囲指定*/
void CMfMediaPlayer::setDisplayArea(const RECT absoluteRect)
{
	workOutNormalisedRect(absoluteRect, &m_normalisedRect);
}
/*表示領域変更*/
bool CMfMediaPlayer::resizeBuffer()
{
	if (m_pMfEngineEx != nullptr && m_hRetWnd != nullptr)
	{
		BOOL iRet = m_pMfEngineEx->HasVideo();
		if (iRet)
		{
			RECT rc;
			::GetClientRect(m_hRetWnd, &rc);
			int iClientWidth = rc.right - rc.left;
			int iClientHeight = rc.bottom - rc.top;

			MFARGB bg{ 0, 0, 0, 0 };
			HRESULT hr = m_pMfEngineEx->UpdateVideoStream(&m_normalisedRect, &rc, &bg);
			return SUCCEEDED(hr);
		}
	}

	return false;
}
/*正規化矩形計算*/
void CMfMediaPlayer::workOutNormalisedRect(const RECT absoluteRect, MFVideoNormalizedRect* normalisedRect)
{
	if (m_pMfEngineEx != nullptr && normalisedRect != nullptr && m_hRetWnd != nullptr)
	{
		BOOL iRet = m_pMfEngineEx->HasVideo();
		if (iRet)
		{
			RECT rc;
			::GetClientRect(m_hRetWnd, &rc);
			int iClientWidth = rc.right - rc.left;
			int iClientHeight = rc.bottom - rc.top;

			float x1 = static_cast<float>(absoluteRect.left) / iClientWidth;
			float y1 = static_cast<float>(absoluteRect.top) / iClientHeight;
			float x2 = static_cast<float>(absoluteRect.right) / iClientWidth;
			float y2 = static_cast<float>(absoluteRect.bottom) / iClientHeight;

			*normalisedRect = MFVideoNormalizedRect{ x1, y1, x2, y2 };
		}
	}
}
