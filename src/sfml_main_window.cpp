

#include "sfml_main_window.h"

CSfmlMainWindow::CSfmlMainWindow(const wchar_t* swzWindowName)
{
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({ 200, 200 }), swzWindowName, sf::Style::None);

	m_window->setPosition(sf::Vector2i(0, 0));

	m_sfmlSpinePlayer = std::make_unique<CSfmlSpinePlayer>();
	m_spineRenderTexture.setSmooth(true);
}

CSfmlMainWindow::~CSfmlMainWindow()
{

}

bool CSfmlMainWindow::setSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	bool bRet = m_sfmlSpinePlayer->loadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
	if (bRet)
	{
		/* Fit the size of spine player to the slot which is assumed to be background. */
		const std::vector<std::string>& slotNames = m_sfmlSpinePlayer->getSlotNames();
		if (!slotNames.empty())
		{
			for (size_t i = 0; i < slotNames.size() || i < 3; ++i)
			{
				const auto& slotName = slotNames[i];
				static constexpr const char bgSlotPrefix[] = "bg";
				static constexpr size_t bgPrefixLength = sizeof(bgSlotPrefix) - 1;
#ifdef _WIN32
				int iRet = _strnicmp(slotName.c_str(), bgSlotPrefix, bgPrefixLength);
#else
				int iRet = strncasecmp(slotName.c_str(), bgSlotPrefix, bgPrefixLength);
#endif
				if (iRet != 0)continue;

				const std::optional<sf::FloatRect>& bgSlotBoundingBox = m_sfmlSpinePlayer->getCurrentBoundingOfSlot(slotName);
				if (bgSlotBoundingBox && bgSlotBoundingBox->size.x > 1024 && bgSlotBoundingBox->size.y > 768 && bgSlotBoundingBox->size.x > bgSlotBoundingBox->size.y * 1.25f)
				{
					m_sfmlSpinePlayer->setBaseSize(bgSlotBoundingBox->size.x, bgSlotBoundingBox->size.y);
					m_sfmlSpinePlayer->update(0.f);
					const auto& updatedSlotBounding = m_sfmlSpinePlayer->getCurrentBoundingOfSlot(slotName);
					if (updatedSlotBounding)
					{
						auto offsetToBe = m_sfmlSpinePlayer->getOffset();
						offsetToBe.x += updatedSlotBounding->position.x;
						offsetToBe.y += updatedSlotBounding->position.y;
						m_sfmlSpinePlayer->setOffset(offsetToBe.x, offsetToBe.y);
						m_sfmlSpinePlayer->setBaseSize(bgSlotBoundingBox->size.x, bgSlotBoundingBox->size.y);
						break;
					}
				}
			}
		}

		/* Filename including extension. */
		size_t nPos = atlasPaths[0].find_last_of("\\/");
		if (nPos == std::string::npos)nPos = 0;
		else ++nPos;
		m_window->setTitle(&atlasPaths[0][nPos]);
	}

	return bRet;
}

void CSfmlMainWindow::setSlotExclusionCallback(bool(*pFunc)(const char*, size_t))
{
	m_sfmlSpinePlayer->setSlotExcludeCallback(pFunc);
}

int CSfmlMainWindow::display()
{
	resetScale();

	updateMessageText();

	sf::Vector2i iMouseStartPos;

	bool toMoveWIndow = false;
	bool wasLeftPressed = false;
	bool wasLeftCombinated = false;

	int iRet = 0;

	m_spineClock.restart();
	while (m_window->isOpen())
	{
		m_window->handleEvents
		(
			[&](const sf::Event::Closed&)
			{
				m_window->close();
			},
			[&](const sf::Event::MouseButtonPressed& event)
			{
				if (event.button == sf::Mouse::Button::Left)
				{
					iMouseStartPos = event.position;

					wasLeftPressed = true;
				}
			},
			[&](const sf::Event::MouseButtonReleased& event)
			{
				if (event.button == sf::Mouse::Button::Left)
				{
					if (wasLeftCombinated)
					{
						wasLeftCombinated = false;
						wasLeftPressed = false;
						return;
					}

					if (toMoveWIndow || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
					{
						toMoveWIndow ^= true;
						return;
					}

					int iX = iMouseStartPos.x - event.position.x;
					int iY = iMouseStartPos.y - event.position.y;

					if (iX == 0 && iY == 0)
					{
						shiftAnimation();
					}

					wasLeftPressed = false;
				}
				else if (event.button == sf::Mouse::Button::Middle)
				{
					m_sfmlSpinePlayer->resetScale();
					resizeWindow();
				}
			},
			[&](const sf::Event::MouseMoved& event)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (wasLeftPressed)
					{
						int iX = iMouseStartPos.x - event.position.x;
						int iY = iMouseStartPos.y - event.position.y;
						m_sfmlSpinePlayer->addOffset(iX, iY);

						iMouseStartPos = event.position;

						wasLeftCombinated = true;
					}
				}
			},
			[&](const sf::Event::MouseWheelScrolled& event)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					static constexpr float kTimeScaleDelta = 0.05f;
					const float scrollSign = event.delta < 0 ? 1.f : -1.f;

					float timeScale = m_sfmlSpinePlayer->getTimeScale() + kTimeScaleDelta * scrollSign;
					timeScale = (std::max)(timeScale, 0.f);
					m_sfmlSpinePlayer->setTimeScale(timeScale);

					wasLeftCombinated = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
				{
					shiftMessageText(event.delta < 0);
				}
				else
				{
					static constexpr float kMinScale = 0.15f;
					const float scrollSign = event.delta < 0 ? 1.f : -1.f;

					float skeletonScale = m_sfmlSpinePlayer->getSkeletonScale() + kScaleDelta * scrollSign;
					skeletonScale = (std::max)(kMinScale, skeletonScale);
					m_sfmlSpinePlayer->setSkeletonScale(skeletonScale);

					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					{
						float canvasScale = m_sfmlSpinePlayer->getCanvasScale() + kScaleDelta * scrollSign;
						canvasScale = (std::max)(kMinScale, canvasScale);
						m_sfmlSpinePlayer->setCanvasScale(canvasScale);

						resizeWindow();
					}
				}
			},
			[&](const sf::Event::KeyPressed& event)
			{
				switch (event.scancode)
				{
				case sf::Keyboard::Scancode::Left:
					shiftMessageText(false);
					break;
				case sf::Keyboard::Scancode::Right:
					if (m_nTextIndex < m_textData.size() - 1)
					{
						shiftMessageText(true);
					}
					break;
				default:
					break;
				}
			},
			[&](const sf::Event::KeyReleased& event)
			{
				switch (event.scancode)
				{
				case sf::Keyboard::Scancode::B:
					m_sfmlSpinePlayer->toggleBlendModeAdoption();
					break;
				case sf::Keyboard::Scancode::C:
					toggleTextColor();
					break;
				case sf::Keyboard::Scancode::H:
					m_isHelpHidden ^= true;
					break;
				case sf::Keyboard::Scancode::R:
					m_sfmlSpinePlayer->setDrawOrder(!m_sfmlSpinePlayer->isDrawOrderReversed());
					break;
				case sf::Keyboard::Scancode::S:
					saveCurrentFrameImage();
					break;
				case sf::Keyboard::Scancode::T:
					toggleTextVisibility();
					break;
				case sf::Keyboard::Scancode::Q:
					toggleImageSync();
					break;
				case sf::Keyboard::Scancode::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Scancode::Up:
					iRet = 2;
					break;
				case sf::Keyboard::Scancode::Down:
					iRet = 1;
					break;
				default:
					break;
				}
			}
		);

		if (iRet != 0)break;

		float fDelta = m_spineClock.getElapsedTime().asSeconds();
		m_sfmlSpinePlayer->update(fDelta);
		m_spineClock.restart();

		m_window->clear(sf::Color(0, 0, 0, 0));
		{
			m_spineRenderTexture.clear(sf::Color(0, 0, 0, 0));
			m_sfmlSpinePlayer->redraw(&m_spineRenderTexture);
			m_spineRenderTexture.display();

			sf::Sprite spineSprite(m_spineRenderTexture.getTexture());
			m_window->draw(spineSprite);
		}

		if (!m_isTextHidden)
		{
			m_window->draw(*m_msgText);
		}

		if (!m_isHelpHidden)
		{
			m_window->draw(*m_helpText);
		}

		m_window->display();

		checkTimer();

		if (toMoveWIndow)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}

	return iRet;
}

void CSfmlMainWindow::resizeWindow()
{
	if (m_sfmlSpinePlayer.get() != nullptr)
	{
		sf::Vector2f fBaseSize = m_sfmlSpinePlayer->getBaseSize();
		float fScale = m_sfmlSpinePlayer->getCanvasScale();

		unsigned int maxWindowWidth = static_cast<unsigned int>(fBaseSize.x * (fScale - kScaleDelta));
		unsigned int maxWindowHeight = static_cast<unsigned int>(fBaseSize.y * (fScale - kScaleDelta));

		const sf::Vector2u &desktopSize = sf::VideoMode::getDesktopMode().size;
		if (maxWindowWidth < desktopSize.x || maxWindowHeight < desktopSize.y)
		{
			unsigned int windowWidth = static_cast<unsigned int>(fBaseSize.x * fScale);
			unsigned int windowHeight = static_cast<unsigned int>(fBaseSize.y * fScale);

			m_window->setSize(sf::Vector2u(windowWidth, windowHeight));
			m_window->setView(sf::View((fBaseSize * fScale) / 2.f, fBaseSize * fScale));

			bool bRet = m_spineRenderTexture.resize({ windowWidth, windowHeight });
		}

		sf::Vector2u windowSize = m_window->getSize();
		sf::FloatRect bounds = m_helpText->getGlobalBounds();
		m_helpText->setPosition(sf::Vector2f{ 0, windowSize.y - bounds.size.y });
	}
}

void CSfmlMainWindow::resetScale()
{
	m_sfmlSpinePlayer->resetScale();
	m_sfmlSpinePlayer->setCanvasScale(m_sfmlSpinePlayer->getSkeletonScale() * 0.9f);
	resizeWindow();
}

bool CSfmlMainWindow::saveCurrentFrameImage()
{
	/* SFML provides neither API to get module path nor to get window title */

	float fTrackTime = 0.f;
	m_sfmlSpinePlayer->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	const char* pzAnimationName = m_sfmlSpinePlayer->getCurrentAnimationName();
	if (pzAnimationName == nullptr)return false;

	std::string strFilePath = pzAnimationName;
	char sBuffer[16]{};
	sprintf_s(sBuffer, "_%.3f.png", fTrackTime);
	strFilePath += sBuffer;

	sf::Texture texture = m_spineRenderTexture.getTexture();
	sf::Image image = texture.copyToImage();

	return image.saveToFile(strFilePath);
}
/*字体設定*/
bool CSfmlMainWindow::setFont(const std::string& strFilePath, bool bold, bool italic)
{
	bool bRet = m_font.openFromFile(strFilePath);
	if (!bRet)return false;

	static constexpr unsigned int fontSize = 42;
	static constexpr float fOutLineThickness = 2.4f;

	m_msgText = std::make_unique<sf::Text>(m_font);
	m_msgText->setFillColor(sf::Color::Black);
	m_msgText->setStyle((bold ? sf::Text::Style::Bold : 0) | (italic ? sf::Text::Style::Italic : 0));
	m_msgText->setCharacterSize(fontSize);
	m_msgText->setOutlineThickness(fOutLineThickness);
	m_msgText->setLineSpacing(0.8f);
	m_msgText->setOutlineColor(sf::Color::White);

	m_helpText = std::make_unique<sf::Text>(*m_msgText);
	m_helpText->setStyle(sf::Text::Regular);
	m_helpText->setCharacterSize(fontSize / 2);

	static constexpr char32_t help[] =
		U"[H] Hide help\n"
		U"[T] Hide message\n"
		U"[C] Toggle text colour\n"
		U"[Scroll] Scale up/down\n"
		U"[Ctrl + scroll] Zoom in/out\n"
		U"[L + scroll] Speed up/down the animation\n"
		U"[L-drag] Move view-point\n"
		U"[M-click] Reset scale, speed, view-point\n"
		U"[R + L-click] Move window\n"
		U"[B] Force blend-mode-normal (* Apply if too-bright)\n"
		U"[S] Save image\n"
		U"[↑ | ↓] Open the next/prev. folder\n"
		U"[← | →; R + scroll] Fast forward/rewind the message\n\n";
	m_helpText->setString(help);

	return true;
}

void CSfmlMainWindow::setScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	m_textData = std::move(textData);
	m_animationNames = std::move(animationNames);

	m_nTextIndex = 0;
	m_msgText->setString("");
	m_nLastAnimationIndex = 0;

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
void CSfmlMainWindow::toggleTextColor()
{
	m_msgText->setFillColor(m_msgText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_msgText->setOutlineColor(m_msgText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);

	m_helpText->setFillColor(m_helpText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_helpText->setOutlineColor(m_helpText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}

void CSfmlMainWindow::toggleTextVisibility()
{
	m_isTextHidden ^= true;
}
void CSfmlMainWindow::toggleImageSync()
{
	m_isImageSynced ^= true;
	if (m_isImageSynced)
	{
		checkAnimationTrack();
	}
}
/*文章切り替え是否確認*/
void CSfmlMainWindow::checkTimer()
{
	constexpr float fAutoPlayInterval = 2.f;
	float fSecond = m_textClock.getElapsedTime().asSeconds();
	if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer.get()->IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			shiftMessageText(true);
		}
		else
		{
			m_textClock.restart();
		}
	}
}
/*表示文章移行*/
void CSfmlMainWindow::shiftMessageText(bool forward)
{
	if (forward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	updateMessageText();
}
/*表示文章変更適用*/
void CSfmlMainWindow::updateMessageText()
{
	if (m_textData.empty())return;

	const adv::TextDatum& textDatum = m_textData.at(m_nTextIndex);
	std::wstring wstr = textDatum.wstrText;

	static constexpr size_t kLineThreashold = 30;
	for (size_t i = kLineThreashold; i < wstr.size(); i += kLineThreashold)
	{
		wstr.insert(i, L"\n");
	}
	if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";

	wchar_t sBuffer[64]{}; /* size_t is 20 digits at most. */
	swprintf(sBuffer, L"%zu/%zu", m_nTextIndex + 1, m_textData.size());
	wstr += sBuffer;
	m_msgText->setString(wstr);

	/* Checks if animation has to be switched or not. */
	if (m_nTextIndex == 0 || (m_nLastAnimationIndex != textDatum.nAnimationIndex))
	{
		if (textDatum.nAnimationIndex < m_animationNames.size())
		{
			m_nLastAnimationIndex = textDatum.nAnimationIndex;
			m_sfmlSpinePlayer->setAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
		}
	}

	if (!textDatum.wstrVoicePath.empty())
	{
		if (m_pAudioPlayer.get() != nullptr)
		{
			m_pAudioPlayer->Play(textDatum.wstrVoicePath.c_str());
		}
	}
	m_textClock.restart();
}

void CSfmlMainWindow::shiftAnimation()
{
	if (m_animationNames.empty())
	{
		m_sfmlSpinePlayer->shiftAnimation();
	}
	else if (!m_isImageSynced)
	{
		if (++m_nLastAnimationIndex >= m_animationNames.size())
		{
			m_nLastAnimationIndex = 0;
		}

		m_sfmlSpinePlayer->setAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
	}
}

void CSfmlMainWindow::checkAnimationTrack()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	if (m_nLastAnimationIndex != textDatum.nAnimationIndex)
	{
		if (textDatum.nAnimationIndex < m_animationNames.size())
		{
			m_nLastAnimationIndex = textDatum.nAnimationIndex;
			m_sfmlSpinePlayer->setAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
		}
	}
}