﻿#ifndef SPINE_SFML_H_
#define SPINE_SFML_H_

#include <spine/spine.h>
#include <SFML/Graphics.hpp>

class CSfmlSpineDrawable : public sf::Drawable
{
public:
	CSfmlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pStateData = nullptr);
	~CSfmlSpineDrawable();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* state = nullptr;
	float timeScale = 1.f;

	void Update(float fDelta);
	/*virtual function from sf::Drawable*/
	virtual void draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const;

	void SwitchPma() { m_bAlphaPremultiplied ^= true; };
	void SwitchBlendModeAdoption() { m_bForceBlendModeNormal ^= true; }

	void SetLeaveOutList(const std::vector<std::string>& list);
private:
	bool m_bHasOwnAnimationStateData = false;
	bool m_bAlphaPremultiplied = false;
	bool m_bForceBlendModeNormal = false;

	mutable spine::Vector<float> m_worldVertices;

	mutable sf::VertexArray m_sfmlVertices;
	/*SFML does not have indices.*/

	mutable spine::SkeletonClipping m_clipper;

	mutable spine::Vector<unsigned short> m_quadIndices;
	
	mutable spine::Vector<spine::String> m_leaveOutList;
	bool IsToBeLeftOut(const spine::String &slotName) const;
};

class CSfmlTextureLoader : public spine::TextureLoader
{
public:
	virtual void load(spine::AtlasPage& page, const spine::String& path);

	virtual void unload(void* texture);
};

#endif /* SPINE_SFML_H_ */
