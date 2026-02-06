#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include"spine_player.h"

class CSfmlSpinePlayer : public CSpinePlayer
{
public:
	CSfmlSpinePlayer();
	virtual ~CSfmlSpinePlayer();

	void redraw(sf::RenderTarget* pRenderTarget);

	std::optional<sf::FloatRect> getCurrentBoundingOfSlot(std::string_view slotName) const;
private:
	void workOutDefaultScale() override;
	void workOutDefaultOffset() override;
};

#endif // !SFML_SPINE_PLAYER_H_
