

#include "sfml_spine_player.h"

CSfmlSpinePlayer::CSfmlSpinePlayer()
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}


void CSfmlSpinePlayer::redraw(sf::RenderTarget* pRenderTarget)
{
	if (pRenderTarget != nullptr)
	{
		const auto& targetSize = pRenderTarget->getSize();
		float fX = (m_fBaseSize.x * m_fSkeletonScale - targetSize.x) / 2;
		float fY = (m_fBaseSize.y * m_fSkeletonScale - targetSize.y) / 2;

		sf::RenderStates renderState;
		renderState.blendMode = sf::BlendAlpha;
		renderState.transform.translate({ -fX, -fY }).scale({ m_fSkeletonScale, m_fSkeletonScale });

		if (!m_isDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				pRenderTarget->draw(*m_drawables[i], renderState);
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				pRenderTarget->draw(*m_drawables[i], renderState);
			}
		}
	}
}

sf::FloatRect CSfmlSpinePlayer::getCurrentBoundingBox() const
{
	float fMinX = std::numeric_limits<float>::max();
	float fMinY = std::numeric_limits<float>::max();
	float fMaxX = std::numeric_limits<float>::lowest();
	float fMaxY = std::numeric_limits<float>::lowest();

	for (const auto& drawable : m_drawables)
	{
		const sf::FloatRect rect = drawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.position.x);
		fMinY = (std::min)(fMinY, rect.position.y);
		fMaxX = (std::max)(fMaxX, rect.position.x + rect.size.x);
		fMaxY = (std::max)(fMaxY, rect.position.y + rect.size.y);
	}

	return { {fMinX, fMinY}, {fMaxX - fMinX, fMaxY - fMinY} };
}

std::optional<sf::FloatRect> CSfmlSpinePlayer::getCurrentBoundingBoxOfSlot(std::string_view slotName) const
{
	bool found = false;
	for (const auto& drawable : m_drawables)
	{
		const auto& rect = drawable->getBoundingBoxOfSlot(slotName.data(), slotName.length(), &found);
		if (found)
		{
			return rect;
		}
	}

	return std::nullopt;
}

void CSfmlSpinePlayer::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	unsigned int skeletonWidth = static_cast<unsigned int>(m_fBaseSize.x);
	unsigned int skeletonHeight = static_cast<unsigned int>(m_fBaseSize.y);

	unsigned int displayWidth = sf::VideoMode::getDesktopMode().size.x;
	unsigned int displayHeight = sf::VideoMode::getDesktopMode().size.y;

	if (skeletonWidth > displayWidth || skeletonHeight > displayHeight)
	{
		float fScaleX = static_cast<float>(displayWidth) / skeletonWidth;
		float fScaleY = static_cast<float>(displayHeight) / skeletonHeight;

		m_fDefaultScale = fScaleX > fScaleY ? fScaleY : fScaleX;
	}
}

void CSfmlSpinePlayer::workOutDefaultOffset()
{
	float fMinX = std::numeric_limits<float>::max();
	float fMinY = std::numeric_limits<float>::max();

	for (const auto& pDrawable : m_drawables)
	{
		const auto& rect = pDrawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.position.x);
		fMinY = (std::min)(fMinY, rect.position.y);
	}

	m_fDefaultOffset = { fMinX, fMinY };
}
