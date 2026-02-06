

#include <atlbase.h>
#include <mfapi.h>

#include "mf_media_player.h"

#pragma comment (lib,"Mfplat.lib")

CMfMediaPlayerNotify::CMfMediaPlayerNotify(void* pMediaPlayer)
	:m_pPlayer(pMediaPlayer)
{

}

CMfMediaPlayerNotify::~CMfMediaPlayerNotify()
{

}

void CMfMediaPlayerNotify::OnMediaEngineEvent(DWORD Event, DWORD_PTR param1, DWORD param2)
{
	CMfMediaPlayer* pPlayer = static_cast<CMfMediaPlayer*>(m_pPlayer);
	if (pPlayer != nullptr)
	{
		const HWND hWnd = pPlayer->GetRetHwnd();
		const UINT uMsg = pPlayer->GetRetMsg();
		if (hWnd != nullptr && uMsg != 0)
		{
			::PostMessage(hWnd, uMsg, 0, Event);
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
/*再生*/
bool CMfMediaPlayer::Play(const wchar_t* pwzFilePath)
{
	if (m_pMfEngineEx != nullptr)
	{
		if (pwzFilePath != nullptr)
		{
			HRESULT hr = m_pMfEngineEx->SetSource(const_cast<BSTR>(pwzFilePath));
		}
		return SUCCEEDED(m_pMfEngineEx->Play());
	}
	return false;
}
/* ループ指定 */
bool CMfMediaPlayer::SetLoop(bool toLoop)
{
	if (m_pMfEngineEx != nullptr)
	{
		HRESULT hr = m_pMfEngineEx->SetLoop(toLoop ? TRUE : FALSE);

		return SUCCEEDED(hr);
	}

	return false;
}
/* ループ有無 */
bool CMfMediaPlayer::IsLooped()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetLoop() == TRUE;
	}

	return false;
}
/* 消音設定 */
bool CMfMediaPlayer::SetMute(bool toMute)
{
	if (m_pMfEngineEx != nullptr)
	{
		HRESULT hr = m_pMfEngineEx->SetMuted(toMute ? TRUE : FALSE);

		return SUCCEEDED(hr);
	}

	return false;
}
/* 消音是否 */
bool CMfMediaPlayer::IsMuted()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetMuted() == TRUE;
	}

	return false;
}
/*停止・再開*/
bool CMfMediaPlayer::SetPause(bool toPause)
{
	if (m_pMfEngineEx != nullptr)
	{
		HRESULT hr = toPause ? m_pMfEngineEx->Pause() : m_pMfEngineEx->Play();
		if (SUCCEEDED(hr))
		{
			if (toPause)
			{
				return FrameStep(true);
			}
		}
	}

	return false;
}
/* 停止是否 */
bool CMfMediaPlayer::IsPaused()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->IsPaused() == TRUE;
	}

	return false;
}
/* コマ送り・戻し */
bool CMfMediaPlayer::FrameStep(bool forward)
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->FrameStep(forward ? TRUE : FALSE) == TRUE;
	}

	return false;
}
/*音量取得*/
double CMfMediaPlayer::GetCurrentVolume()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetVolume();
	}
	return 100.0;
}
/*再生速度取得*/
double CMfMediaPlayer::GetCurrentRate()
{
	if (m_pMfEngineEx != nullptr)
	{
		return m_pMfEngineEx->GetPlaybackRate();
	}
	return 1.0;
}
/*再生位置取得*/
long long CMfMediaPlayer::GetCurrentTimeInMilliSeconds()
{
	if (m_pMfEngineEx != nullptr)
	{
		double dbTime = m_pMfEngineEx->GetCurrentTime();
		return static_cast<long long>(::round(dbTime * 1000));
	}

	return 0;
}
/*音量設定*/
bool CMfMediaPlayer::SetCurrentVolume(double dbVolume)
{
	if (m_pMfEngineEx != nullptr)
	{
		return SUCCEEDED(m_pMfEngineEx->SetVolume(dbVolume));
	}
	return false;
}
/*再生速度設定*/
bool CMfMediaPlayer::SetCurrentRate(double dbRate)
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
/*再生中是否*/
bool CMfMediaPlayer::IsEnded()
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
/*発生事象通知先・描画先指定。*/
bool CMfMediaPlayer::SetPlaybackWindow(HWND hWnd, UINT uMsg)
{
	m_hRetWnd = hWnd;
	m_uRetMsg = uMsg;

	HRESULT hr = m_pMfAttributes->SetUINT64(MF_MEDIA_ENGINE_PLAYBACK_HWND, reinterpret_cast<UINT64>(m_hRetWnd));
	return SUCCEEDED(hr);
}
/*動画縦横幅取得*/
bool CMfMediaPlayer::GetVideoSize(DWORD* dwWidth, DWORD* dwHeight)
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
void CMfMediaPlayer::SetDisplayArea(const RECT absoluteRect)
{
	WorkOutNormalisedRect(absoluteRect, &m_normalisedRect);
}
/*表示領域変更*/
bool CMfMediaPlayer::ResizeBuffer()
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
void CMfMediaPlayer::WorkOutNormalisedRect(const RECT absoluteRect, MFVideoNormalizedRect* normalisedRect)
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
