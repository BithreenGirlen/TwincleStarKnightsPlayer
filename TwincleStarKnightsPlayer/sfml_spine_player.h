#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include <memory>

#include "sfml_spine.h"
#include "adv.h"

/*Windows OS*/
#include "mf_media_player.h"

class CSfmlSpinePlayer
{
public:
	CSfmlSpinePlayer();
	~CSfmlSpinePlayer();
	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);
	int Display(const wchar_t* pwzWindowName);
private:
	enum Size { kBaseWidth = 1280, kBaseHeight = 700, kMinAtlas };

	CSfmlTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSfmlSpineDrawer>> m_drawables;

	std::unique_ptr<sf::RenderWindow> m_window;
	sf::Vector2f m_fBaseWindowSize = sf::Vector2f{ Size::kBaseWidth, Size::kBaseHeight };

	const float m_kfScalePortion = 0.025f;

	float m_fDefaultWindowScale = 1.f;
	float m_fThresholdScale = 1.f;

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	sf::Vector2i m_iOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	bool m_bDrawOrderReversed = false;

	bool SetupDrawer();
	void WorkOutDefaultScale();

	void RescaleSkeleton(bool bUpscale);
	void RescaleTime(bool bHasten);
	void UpdateScaletonScale();
	void UpdateTimeScale();
	void ResetScale();
	void ResizeWindow();

	void MoveViewPoint(int iX, int iY);
	void ShiftAnimation();

	void ShiftSkin(bool bForward);
	void UpdateSkin();

	void Redraw(float fDelta);

	/*ï∂èÕï\é¶óp*/
public:
	bool SetFont(const std::string& strFilePath, bool bBold, bool bItalic);
	void SetTexts(const std::vector<adv::TextDatum>& textData);
private:
	sf::Font m_font;
	sf::Text m_msgText;
	sf::Clock m_clock;

	size_t m_nTextIndex = 0;
	std::vector<adv::TextDatum> m_textData;
	bool m_bTextHidden = false;

	void SwitchTextColor();

	void CheckTimer();
	void ShiftMessageText(bool bForward);
	void UpdateMessageText();

	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
	void ChangePlaybackRate(bool bFaster);
	void ResetPlacybackRate();
};

#endif // SFML_SPINE_PLAYER_H_
