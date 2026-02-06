#ifndef SFML_SPINE_CPP_H_
#define SFML_SPINE_CPP_H_

#include <spine/spine.h>
#include <SFML/Graphics.hpp>

class CSfmlSpineDrawable : public sf::Drawable
{
public:
	CSfmlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pStateData = nullptr);
	~CSfmlSpineDrawable();

	spine::Skeleton* skeleton() const noexcept;
	spine::AnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void update(float fDelta);
	void draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const override;

	void setLeaveOutList(spine::Vector<spine::String>& list);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	sf::FloatRect getBoundingBox() const;
	sf::FloatRect getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr) const;
private:
	bool m_hasOwnAnimationStateData = false;
	mutable bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;

	spine::Skeleton* m_skeleton = nullptr;
	spine::AnimationState* m_animationState = nullptr;

	mutable spine::SkeletonClipping m_clipper;

	mutable spine::Vector<float> m_worldVertices;
	mutable sf::VertexArray m_sfmlVertices;
	/*SFML does not have indices.*/
	mutable spine::Vector<unsigned short> m_quadIndices;
	
	mutable spine::Vector<spine::String> m_leaveOutList;
	bool isSlotToBeLeftOut(const spine::String &slotName) const;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;
};

class CSfmlTextureLoader : public spine::TextureLoader
{
public:
	void load(spine::AtlasPage& page, const spine::String& path) override;

	void unload(void* texture) override;
};

#endif //!SFML_SPINE_CPP_H_
