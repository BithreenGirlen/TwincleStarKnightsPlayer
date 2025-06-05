#ifndef SPINE_PLAYER_H_
#define SPINE_PLAYER_H_

/*Base-type spine player regardless of rendering library.*/

#include <string>
#include <vector>
#include <memory>


#include "sfml_spine.h"
using FPoint2 = sf::Vector2f;
using CSpineDrawable = CSfmlSpineDrawer;
using CTextureLoader = CSfmlTextureLoader;

class CSpinePlayer
{
public:
	CSpinePlayer();
	virtual ~CSpinePlayer();

	bool LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	virtual void Update(float fDelta);
	virtual void Redraw() = 0;

	void RescaleSkeleton(bool bUpscale);
	void RescaleCanvas(bool bUpscale);
	void RescaleTime(bool bHasten);

	void ResetScale();

	void MoveViewPoint(int iX, int iY);

	void ShiftAnimation();
	void ShiftSkin();

	void TogglePma();
	void ToggleBlendModeAdoption();
	void ToggleDrawOrder();

	const char* GetCurrentAnimationNameWithTrackTime(float* fTrackTime = nullptr);

	std::vector<std::string> GetSlotList();
	const std::vector<std::string>& GetSkinList() const;
	const std::vector<std::string>& GetAnimationList() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);

	void SetSlotExclusionCallback(bool (*pFunc)(const char*, size_t));

	FPoint2 GetBaseSize() const;
	float GetCanvasScale() const;
protected:
	static constexpr float kfScalePortion = 0.025f;
	static constexpr float kfMinScale = 0.15f;
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	CTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSpineDrawable>> m_drawables;

	FPoint2 m_fBaseSize = FPoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	FPoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	FPoint2 m_fOffset{};

	bool m_bDrawOrderReversed = false;

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	void ClearDrawables();
	bool SetupDrawer();

	void WorkOutDefaultSize();
	virtual void WorkOutDefaultScale() = 0;
	virtual void WorkOutDefaultOffset() = 0;

	void UpdatePosition();
	void UpdateTimeScale();

	void RestartAnimation();
	void ClearAnimationTracks();
};

#endif // !SPINE_PLAYER_H_
