#ifndef SFML_MAIN_WINDOW_H_
#define SFML_MAIN_WINDOW_H_

#include <memory>

#include "sfml-spine-cpp/sfml_spine_player.h"

#include "adv.h"
/* Rely on Media Foundation to play AAC. */
#include "mf_media_player.h"

class CSfmlMainWindow
{
public:
	CSfmlMainWindow(const wchar_t* swzWindowName = nullptr);
	~CSfmlMainWindow();

	bool setSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel);

	void setSlotExclusionCallback(bool (*pFunc)(const char*, size_t));

	int display();

	sf::RenderWindow* getWindow() const { return m_window.get(); }
private:
	static constexpr float kScaleDelta = 0.025f;

	std::unique_ptr<sf::RenderWindow> m_window;

	std::unique_ptr<CSfmlSpinePlayer> m_sfmlSpinePlayer;
	sf::Clock m_spineClock;
	sf::RenderTexture m_spineRenderTexture;

	void resizeWindow();
	void resetScale();

	bool saveCurrentFrameImage();

	/*文章表示用*/
public:
	bool setFont(const std::string& strFilePath, bool bold, bool italic);
	void setScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames);
private:
	sf::Font m_font;
	std::unique_ptr<sf::Text> m_msgText;
	sf::Clock m_textClock;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;
	bool m_isTextHidden = false;
	bool m_isImageSynced = true;

	bool m_isHelpHidden = false;
	std::unique_ptr<sf::Text> m_helpText;

	std::vector<std::string> m_animationNames;
	size_t m_nLastAnimationIndex = 0;

	void toggleTextColor();
	void toggleTextVisibility();
	void toggleImageSync();

	void checkTimer();
	void shiftMessageText(bool forward);
	void updateMessageText();

	void shiftAnimation();
	void checkAnimationTrack();

	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
};

#endif // !SFML_MAIN_WINDOW_H_
