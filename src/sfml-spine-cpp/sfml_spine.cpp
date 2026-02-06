

#include "sfml_spine.h"

namespace spine
{
	SpineExtension* getDefaultExtension()
	{
		return new DefaultSpineExtension();
	}
}

static sf::BlendMode g_sfmlBlendModeNormalPma = sf::BlendMode(sf::BlendMode::Factor::One, sf::BlendMode::Factor::OneMinusSrcAlpha);
static sf::BlendMode g_sfmlBlendModeAddPma = sf::BlendMode(sf::BlendMode::Factor::One, sf::BlendMode::Factor::One);
static sf::BlendMode g_sfmlBlendModeScreen = sf::BlendMode(sf::BlendMode::Factor::One, sf::BlendMode::Factor::OneMinusSrcColor);
static sf::BlendMode g_sfmlBlendModeMultiply = sf::BlendMode
(
	sf::BlendMode::Factor::DstColor,
	sf::BlendMode::Factor::OneMinusSrcAlpha,
	sf::BlendMode::Equation::Add,
	sf::BlendMode::Factor::One,
	sf::BlendMode::Factor::OneMinusSrcAlpha,
	sf::BlendMode::Equation::Add
);

CSfmlSpineDrawable::CSfmlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	/*sf::VertexArray seems not to have reserve-like method.*/
	m_sfmlVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

	m_skeleton = new spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new spine::AnimationStateData(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}

	m_animationState = new spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);
}

CSfmlSpineDrawable::~CSfmlSpineDrawable()
{
	if (m_animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
		{
			delete m_animationState->getData();
		}

		delete m_animationState;
	}
	if (m_skeleton != nullptr)
	{
		delete m_skeleton;
	}
}

spine::Skeleton* CSfmlSpineDrawable::skeleton() const noexcept
{
	return m_skeleton;
}

spine::AnimationState* CSfmlSpineDrawable::animationState() const noexcept
{
	return m_animationState;
}

void CSfmlSpineDrawable::premultiplyAlpha(bool premultiplied) noexcept
{
	m_isAlphaPremultiplied = premultiplied;
}

bool CSfmlSpineDrawable::isAlphaPremultiplied() const noexcept
{
	return m_isAlphaPremultiplied;
}

void CSfmlSpineDrawable::forceBlendModeNormal(bool toForce) noexcept
{
	m_isToForceBlendModeNormal = toForce;
}

bool CSfmlSpineDrawable::isBlendModeNormalForced() const noexcept
{
	return m_isToForceBlendModeNormal;
}

void CSfmlSpineDrawable::update(float fDelta)
{
	if (m_skeleton != nullptr && m_animationState != nullptr)
	{
#ifndef SPINE_4_1_OR_LATER
		m_skeleton->update(fDelta);
#endif
		m_animationState->update(fDelta);
		m_animationState->apply(*m_skeleton);
#ifdef SPINE_4_2_OR_LATER
		m_skeleton->update(fDelta);
		m_skeleton->updateWorldTransform(spine::Physics::Physics_Update);
#else
		m_skeleton->updateWorldTransform();
#endif
	}
}

void CSfmlSpineDrawable::draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const
{
	if (m_skeleton == nullptr || m_animationState == nullptr)return;

	if (m_skeleton->getColor().a == 0) return;

	for (unsigned i = 0; i < m_skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *m_skeleton->getDrawOrder()[i];
		spine::Attachment* pAttachment = slot.getAttachment();

		if (pAttachment == nullptr || slot.getColor().a == 0 || !slot.getBone().isActive())
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (isSlotToBeLeftOut(slot.getData().getName()))
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float>* pVertices = &m_worldVertices;
		spine::Vector<float>* pAttachmentUvs = nullptr;
		spine::Vector<unsigned short>* pIndices = nullptr;

		spine::Color* pAttachmentColor = nullptr;

		sf::Texture* pSfmlTexture = nullptr;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
			pAttachmentColor = &pRegionAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
			pRegionAttachment->computeWorldVertices(slot, m_worldVertices, 0, 2);
#else
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
#endif
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRegion());

			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject());
#ifdef SPINE_4_0
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
			pAttachmentColor = &pMeshAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);

			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRegion());

			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject());
#ifdef SPINE_4_0
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment* clip = (spine::ClippingAttachment*)slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else continue;

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			pVertices = &m_clipper.getClippedVertices();
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
		}

		const spine::Color& skeletonColor = m_skeleton->getColor();
		const spine::Color& slotColor = slot.getColor();
		const spine::Color tint
		(
			skeletonColor.r * slotColor.r * pAttachmentColor->r,
			skeletonColor.g * slotColor.g * pAttachmentColor->g,
			skeletonColor.b * slotColor.b * pAttachmentColor->b,
			skeletonColor.a * slotColor.a * pAttachmentColor->a
		);
		const sf::Vector2u& textureSize = pSfmlTexture->getSize();

		m_sfmlVertices.resize(pIndices->size());
		for (int ii = 0; ii < pIndices->size(); ++ii)
		{
			sf::Vertex& sfmlVertex = m_sfmlVertices[ii];

			sfmlVertex.position.x = (*pVertices)[(*pIndices)[ii] * 2LL];
			sfmlVertex.position.y = (*pVertices)[(*pIndices)[ii] * 2LL + 1];

			sfmlVertex.color.r = (std::uint8_t)(tint.r * 255.f * (m_isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.g = (std::uint8_t)(tint.g * 255.f * (m_isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.b = (std::uint8_t)(tint.b * 255.f * (m_isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.a = (std::uint8_t)(tint.a * 255.f);

			sfmlVertex.texCoords.x = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL] * textureSize.x;
			sfmlVertex.texCoords.y = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL + 1] * textureSize.y;
		}

		sf::BlendMode sfmlBlendMode;
		spine::BlendMode spineBlnedMode = m_isToForceBlendModeNormal ? spine::BlendMode_Normal : slot.getData().getBlendMode();
		switch (spineBlnedMode)
		{
		case spine::BlendMode_Additive:
			sfmlBlendMode = m_isAlphaPremultiplied ? g_sfmlBlendModeAddPma : sf::BlendAdd;
			break;
		case spine::BlendMode_Multiply:
			sfmlBlendMode = g_sfmlBlendModeMultiply;
			break;
		case spine::BlendMode_Screen:
			sfmlBlendMode = g_sfmlBlendModeScreen;
			break;
		default:
			sfmlBlendMode = m_isAlphaPremultiplied ? g_sfmlBlendModeNormalPma : sf::BlendAlpha;
			break;
		}

		renderStates.blendMode = sfmlBlendMode;
		renderStates.texture = pSfmlTexture;
		renderTarget.draw(m_sfmlVertices, renderStates);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CSfmlSpineDrawable::setLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clearAndAddAll(list);
}

sf::FloatRect CSfmlSpineDrawable::getBoundingBox() const
{
	sf::FloatRect boundingBox{};

	if (m_skeleton != nullptr)
	{
		spine::Vector<float> tempVertices;
		m_skeleton->getBounds(boundingBox.position.x, boundingBox.position.y, boundingBox.size.x, boundingBox.size.y, tempVertices);
	}

	return boundingBox;
}

sf::FloatRect CSfmlSpineDrawable::getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found) const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	if (m_skeleton != nullptr)
	{
		for (size_t i = 0; i < m_skeleton->getSlots().size(); ++i)
		{
			spine::Slot& slot = *m_skeleton->getDrawOrder()[i];
			const spine::String& slotDataName = slot.getData().getName();
			if (nameLength != slotDataName.length())continue;

			if (::memcmp(slotDataName.buffer(), slotName, slotDataName.length()) == 0)
			{
				spine::Attachment* pAttachment = slot.getAttachment();
				if (pAttachment != nullptr)
				{
					spine::Vector<float> tempVertices;
					if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
					{
						spine::RegionAttachment* pRegionAttachment = static_cast<spine::RegionAttachment*>(pAttachment);

						tempVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
						pRegionAttachment->computeWorldVertices(slot, tempVertices, 0, 2);
#else
						pRegionAttachment->computeWorldVertices(slot.getBone(), tempVertices, 0, 2);
#endif
					}
					else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
					{
						spine::MeshAttachment* pMeshAttachment = static_cast<spine::MeshAttachment*>(pAttachment);
						tempVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
						pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), tempVertices, 0, 2);
					}
					else
					{
						continue;
					}

					for (size_t ii = 0; ii < tempVertices.size(); ii += 2)
					{
						float fX = tempVertices[ii];
						float fY = tempVertices[ii + 1LL];

						fMinX = fMinX < fX ? fMinX : fX;
						fMinY = fMinY < fY ? fMinY : fY;
						fMaxX = fMaxX > fX ? fMaxX : fX;
						fMaxY = fMaxY > fY ? fMaxY : fY;
					}

					if (found != nullptr)*found = true;
					break;
				}
			}
		}
	}

	return sf::FloatRect{ {fMinX, fMinY}, {fMaxX - fMinX, fMaxY - fMinY} };
}

bool CSfmlSpineDrawable::isSlotToBeLeftOut(const spine::String& slotName) const
{
	/*The comparison method depends on what should be excluded; the precise matching or partial one.*/
	if (m_pLeaveOutCallback != nullptr)
	{
		return m_pLeaveOutCallback(slotName.buffer(), slotName.length());
	}
	else
	{
		return m_leaveOutList.contains(slotName);
	}

	return false;
}

void CSfmlTextureLoader::load(spine::AtlasPage& atlasPage, const spine::String& path)
{
	sf::Texture* texture = new sf::Texture();
	if (!texture->loadFromFile(path.buffer()))
	{
		delete texture;
		return;
	}

	if (atlasPage.magFilter == spine::TextureFilter_Linear) texture->setSmooth(true);
	if (atlasPage.uWrap == spine::TextureWrap_Repeat && atlasPage.vWrap == spine::TextureWrap_Repeat) texture->setRepeated(true);

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	if (atlasPage.width == 0 || atlasPage.height == 0)
	{
		sf::Vector2u size = texture->getSize();
		atlasPage.width = size.x;
		atlasPage.height = size.y;
	}

#ifdef SPINE_4_1_OR_LATER
	atlasPage.texture = texture;
#else
	atlasPage.setRendererObject(texture);
#endif
}

void CSfmlTextureLoader::unload(void* texture)
{
	delete static_cast<sf::Texture*>(texture);
}
