

#include "sfml_spine_player.h"
#include "spine_loader.h"

CSfmlSpinePlayer::CSfmlSpinePlayer()
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}

bool CSfmlSpinePlayer::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonPath = skelPaths.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultScale();

	return SetupDrawer();
}

bool CSfmlSpinePlayer::SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData.at(i);
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonData = skelData.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum .size()), strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultScale();

	return SetupDrawer();
}
/*ウィンドウ表示*/
int CSfmlSpinePlayer::Display(const wchar_t* pwzWindowName)
{
	int iRet = 0;

	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(static_cast<unsigned int>(m_fBaseWindowSize.x), static_cast<unsigned int>(m_fBaseWindowSize.y)), pwzWindowName, sf::Style::None);
	m_window->setPosition(sf::Vector2i(0, 0));
	m_window->setFramerateLimit(0);

	ResetScale();
	UpdateMessageText();

	sf::Vector2i iMouseStartPos;

	bool bOnWindowMove = false;
	bool bSpeedHavingChanged = false;

	m_spineClock.restart();
	while (m_window->isOpen())
	{
		sf::Event event;
		while (m_window->pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window->close();
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					iMouseStartPos.x = event.mouseButton.x;
					iMouseStartPos.y = event.mouseButton.y;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (bSpeedHavingChanged)
					{
						bSpeedHavingChanged = false;
						break;
					}

					if (bOnWindowMove || sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						bOnWindowMove ^= true;
						break;
					}

					int iX = iMouseStartPos.x - event.mouseButton.x;
					int iY = iMouseStartPos.y - event.mouseButton.y;

					if (iX == 0 && iY == 0)
					{
						ShiftAnimation();
					}
					else
					{
						MoveViewPoint(iX, iY);
					}
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					ResetScale();
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					RescaleTime(event.mouseWheelScroll.delta < 0);
					bSpeedHavingChanged = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					ShiftMessageText(event.mouseWheelScroll.delta < 0);
				}
				else
				{
					RescaleSkeleton(event.mouseWheelScroll.delta < 0);
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::A:
					for (size_t i = 0; i < m_drawables.size(); ++i)
					{
						m_drawables.at(i).get()->SwitchPma();
					}
					break;
				case sf::Keyboard::Key::B:
					for (size_t i = 0; i < m_drawables.size(); ++i)
					{
						m_drawables.at(i).get()->SwitchBlendModeAdoption();
					}
					break;
				case sf::Keyboard::Key::C:
					SwitchTextColor();
					break;
				case sf::Keyboard::Key::R:
					m_bDrawOrderReversed ^= true;
					break;
				case sf::Keyboard::Key::S:

					break;
				case sf::Keyboard::Key::T:
					m_bTextHidden ^= true;
					break;
				case sf::Keyboard::Key::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Key::PageUp:
					ChangePlaybackRate(true);
					break;
				case sf::Keyboard::Key::PageDown:
					ChangePlaybackRate(false);
					break;
				case sf::Keyboard::Key::Home:
					ResetPlacybackRate();
					break;
				case sf::Keyboard::Key::Up:
					iRet = 2;
					m_window->close();
					break;
				case sf::Keyboard::Key::Down:
					iRet = 1;
					m_window->close();
					break;
				default:
					break;
				}
				break;
			}
		}

		Redraw();

		CheckTimer();

		if (bOnWindowMove)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}
	return iRet;
}
/*消去*/
void CSfmlSpinePlayer::ClearDrawables()
{
	m_drawables.clear();
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;
}
/*描画器設定*/
bool CSfmlSpinePlayer::SetupDrawer()
{
	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		m_drawables.emplace_back(std::make_shared<CSfmlSpineDrawer>(m_skeletonData.at(i).get()));

		CSfmlSpineDrawer* drawable = m_drawables.at(i).get();
		drawable->timeScale = 1.0f;
		drawable->skeleton->setPosition(m_fBaseWindowSize.x / 2, m_fBaseWindowSize.y / 2);
		drawable->skeleton->updateWorldTransform();

		auto& animations = m_skeletonData.at(i).get()->getAnimations();
		for (size_t ii = 0; ii < animations.size(); ++ii)
		{
			const std::string &strAnimationName = animations[ii]->getName().buffer();
			auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}

		auto& skins = m_skeletonData.at(i).get()->getSkins();
		for (size_t ii = 0; ii < skins.size(); ++ii)
		{
			const std::string& strName = skins[ii]->getName().buffer();
			auto iter = std::find(m_skinNames.begin(), m_skinNames.end(), strName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(strName);
		}
	}

	if (!m_animationNames.empty())
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(m_animationNames.at(0).c_str());
			if (animation != nullptr)
			{
				m_drawables.at(i).get()->animationState->setAnimation(0, animation->getName(), true);
			}
		}
	}

	return m_animationNames.size() > 0;
}
/*標準尺度算出*/
void CSfmlSpinePlayer::WorkOutDefaultScale()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> void
		{
			if (fWidth * fHeight > fMaxSize)
			{
				m_fBaseWindowSize.x = fWidth;
				m_fBaseWindowSize.y = fHeight;
				fMaxSize = fWidth * fHeight;
			}
		};

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		if (pSkeletonDatum->getWidth() > 0.f && pSkeletonDatum->getHeight() > 0.f)
		{
			CompareDimention(pSkeletonDatum->getWidth(), pSkeletonDatum->getHeight());
		}
		else
		{
			/*If skeletonData does not store size, deduce from the attachment of the default skin.*/
			const auto FindDefaultSkinAttachment = [&pSkeletonDatum]()
				-> spine::Attachment*
				{
					spine::Skin::AttachmentMap::Entries attachmentMapEntries = pSkeletonDatum->getDefaultSkin()->getAttachments();
					for (; attachmentMapEntries.hasNext();)
					{
						spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();
						if (attachmentMapEntry._slotIndex == 0)
						{
							return attachmentMapEntry._attachment;
						}
					}
					return nullptr;
				};

			spine::Attachment* pAttachment = FindDefaultSkinAttachment();
			if (pAttachment == nullptr)continue;

			if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
			{
				spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
				if (pRegionAttachment->getWidth() > 0.f && pRegionAttachment->getHeight() > 0.f)
				{
					CompareDimention
					(
						pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(),
						pRegionAttachment->getHeight() * pRegionAttachment->getScaleY()
					);
				}
			}
			else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
			{
				spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
				if (pMeshAttachment->getWidth() > 0.f && pMeshAttachment->getHeight() > 0.f)
				{
					float fScale = pMeshAttachment->getWidth() > Size::kMinAtlas && pMeshAttachment->getHeight() > Size::kMinAtlas ? 1.f : 2.f;
					CompareDimention(pMeshAttachment->getWidth() * fScale, pMeshAttachment->getHeight() * fScale);
				}
			}
		}
	}

	unsigned int uiSkeletonWidth = static_cast<unsigned int>(m_fBaseWindowSize.x);
	unsigned int uiSkeletonHeight = static_cast<unsigned int>(m_fBaseWindowSize.y);

	unsigned int uiDesktopWidth = sf::VideoMode::getDesktopMode().width;
	unsigned int uiDesktopHeight = sf::VideoMode::getDesktopMode().height;

	if (uiSkeletonWidth > uiDesktopWidth || uiSkeletonHeight > uiDesktopHeight)
	{
		if (uiDesktopWidth > uiDesktopHeight)
		{
			m_fDefaultWindowScale = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;
			m_fThresholdScale = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
		}
		else
		{
			m_fDefaultWindowScale = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
			m_fThresholdScale = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;
		}
		m_fSkeletonScale = m_fDefaultWindowScale;
	}
}
/*拡縮変更*/
void CSfmlSpinePlayer::RescaleSkeleton(bool bUpscale)
{
	constexpr float kfMinScale = 0.25f;
	if (bUpscale)
	{
		m_fSkeletonScale += m_kfScalePortion;
	}
	else
	{
		m_fSkeletonScale -= m_kfScalePortion;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
	UpdateScaletonScale();
}
/*時間尺度変更*/
void CSfmlSpinePlayer::RescaleTime(bool bHasten)
{
	constexpr float kfTimeScalePortion = 0.05f;
	if (bHasten)
	{
		m_fTimeScale += kfTimeScalePortion;
	}
	else
	{
		m_fTimeScale -= kfTimeScalePortion;
	}
	if (m_fTimeScale < 0.f)m_fTimeScale = 0.f;

	UpdateTimeScale();
}
/*尺度変更適用設定*/
void CSfmlSpinePlayer::UpdateScaletonScale()
{
	float fOffset = m_fSkeletonScale - m_fThresholdScale > 0.f ? m_fSkeletonScale - m_fThresholdScale : 0;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setScaleX(m_fSkeletonScale > 0.99f + fOffset ? m_fSkeletonScale : 1.f + fOffset);
		m_drawables.at(i).get()->skeleton->setScaleY(m_fSkeletonScale > 0.99f + fOffset ? m_fSkeletonScale : 1.f + fOffset);
	}

	unsigned int uiWindowWidthMax = static_cast<unsigned int>(m_fBaseWindowSize.x * (m_fSkeletonScale - m_kfScalePortion));
	unsigned int uiWindowHeightMax = static_cast<unsigned int>(m_fBaseWindowSize.y * (m_fSkeletonScale - m_kfScalePortion));
	if (uiWindowWidthMax < sf::VideoMode::getDesktopMode().width || uiWindowHeightMax < sf::VideoMode::getDesktopMode().height)
	{
		ResizeWindow();
	}
}
/*速度変更適用*/
void CSfmlSpinePlayer::UpdateTimeScale()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->timeScale = m_fTimeScale;
	}
}
/*速度・尺度・視点初期化*/
void CSfmlSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultWindowScale;
	m_iOffset = sf::Vector2i{};

	UpdateScaletonScale();
	UpdateTimeScale();
	MoveViewPoint(0, 0);
	ResizeWindow();
}
/*窓寸法調整*/
void CSfmlSpinePlayer::ResizeWindow()
{
	if (m_window.get() != nullptr)
	{
		m_window->setSize(sf::Vector2u(static_cast<unsigned int>(m_fBaseWindowSize.x * m_fSkeletonScale), static_cast<unsigned int>(m_fBaseWindowSize.y * m_fSkeletonScale)));
	}
}
/*視点移動*/
void CSfmlSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_iOffset.x += iX;
	m_iOffset.y += iY;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setPosition(m_fBaseWindowSize .x/ 2 - m_iOffset.x, m_fBaseWindowSize.y / 2 - m_iOffset.y);
	}
}
/*動作移行*/
void CSfmlSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex > m_animationNames.size() - 1)m_nAnimationIndex = 0;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(m_animationNames.at(m_nAnimationIndex).c_str());
		if (animation != nullptr)
		{
			m_drawables.at(i).get()->animationState->setAnimation(0, animation->getName(), true);
		}
	}
}
/*装い移行*/
void CSfmlSpinePlayer::ShiftSkin(bool bForward)
{
	if (bForward)
	{
		++m_nSkinIndex;
	}
	else
	{
		--m_nSkinIndex;
	}
	if (m_nSkinIndex > m_skinNames.size() - 1)m_nSkinIndex = 0;
	UpdateSkin();
}
/*装い変更適用*/
void CSfmlSpinePlayer::UpdateSkin()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Skin* skin = m_skeletonData.at(i).get()->findSkin(m_skinNames.at(m_nSkinIndex).c_str());
		if (skin != nullptr)
		{
			m_drawables.at(i).get()->skeleton->setSkin(skin);
			m_drawables.at(i).get()->skeleton->setSlotsToSetupPose();
		}
	}
}
/*再描画*/
void CSfmlSpinePlayer::Redraw()
{
	float fDelta = m_spineClock.getElapsedTime().asSeconds();
	m_spineClock.restart();

	if (m_window.get() != nullptr)
	{
		m_window->clear();
		if (!m_bDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_drawables.at(i).get()->Update(fDelta);
				m_window->draw(*m_drawables.at(i).get(), sf::RenderStates(sf::BlendAlpha));
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				m_drawables.at(i).get()->Update(fDelta);
				m_window->draw(*m_drawables.at(i).get(), sf::RenderStates(sf::BlendAlpha));
			}
		}

		if (!m_bTextHidden)
		{
			m_window->draw(m_msgText);
		}
		m_window->display();
	}
}
/*字体設定*/
bool CSfmlSpinePlayer::SetFont(const std::string& strFilePath, bool bBold, bool bItalic)
{
	bool bRet = m_font.loadFromFile(strFilePath);
	if (!bRet)return false;

	constexpr float fOutLineThickness = 2.4f;

	m_msgText.setFont(m_font);
	m_msgText.setFillColor(sf::Color::Black);
	m_msgText.setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_msgText.setOutlineThickness(fOutLineThickness);
	m_msgText.setOutlineColor(sf::Color::White);

	return true;
}
/*文章格納*/
void CSfmlSpinePlayer::SetTexts(const std::vector<adv::TextDatum>& textData)
{
	m_textData = textData;
	m_nTextIndex = 0;
	m_msgText.setString("");

	const auto HasAudio = [](const adv::TextDatum& text)
		-> bool
		{
			return !text.wstrVoicePath.empty();
		};
	const auto iter = std::find_if(m_textData.begin(), m_textData.end(), HasAudio);
	if (iter != m_textData.cend())
	{
		m_pAudioPlayer = std::make_unique<CMfMediaPlayer>();
	}
}
/*文字色切り替え*/
void CSfmlSpinePlayer::SwitchTextColor()
{
	m_msgText.setFillColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_msgText.setOutlineColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}
/*文章切り替え是否確認*/
void CSfmlSpinePlayer::CheckTimer()
{
	constexpr float fAutoPlayInterval = 2.f;
	float fSecond = m_textClock.getElapsedTime().asSeconds();
	if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer.get()->IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
		else
		{
			m_textClock.restart();
		}
	}
}
/*表示文章移行*/
void CSfmlSpinePlayer::ShiftMessageText(bool bForward)
{
	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	UpdateMessageText();
}
/*表示文章変更適用*/
void CSfmlSpinePlayer::UpdateMessageText()
{
	if (m_textData.empty())return;

	const adv::TextDatum& textDatum = m_textData.at(m_nTextIndex);
	std::wstring wstr = textDatum.wstrText;

	constexpr unsigned int kLineThreashold = 46;
	for (size_t i = kLineThreashold; i < wstr.size(); i += kLineThreashold)
	{
		wstr.insert(i, L"\n");
	}
	if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";

	wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	m_msgText.setString(wstr);

	if (!textDatum.wstrVoicePath.empty())
	{
		if (m_pAudioPlayer.get() != nullptr)
		{
			m_pAudioPlayer->Play(textDatum.wstrVoicePath.c_str());
		}
	}
	m_textClock.restart();
}
/*音声再生速度変更*/
void CSfmlSpinePlayer::ChangePlaybackRate(bool bFaster)
{
	if (m_pAudioPlayer.get() != nullptr)
	{
		double dbRate = m_pAudioPlayer.get()->GetCurrentRate();
		constexpr double fRatePortion = 0.1;
		if (bFaster && dbRate < 2.49)
		{
			dbRate += fRatePortion;
		}
		else if (!bFaster && dbRate > 0.51)
		{
			dbRate -= fRatePortion;
		}
		m_pAudioPlayer.get()->SetCurrentRate(dbRate);
	}
}
/*音声再生速度初期化*/
void CSfmlSpinePlayer::ResetPlacybackRate()
{
	if (m_pAudioPlayer.get() != nullptr)
	{
		m_pAudioPlayer.get()->SetCurrentRate(1.0);
	}
}
