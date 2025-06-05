#ifndef ADV_H_
#define ADV_H_

#include <string>

namespace adv
{
	struct TextDatum
	{
		std::wstring wstrText;
		std::wstring wstrVoicePath;

		/* This ought be defined in another struct, but it is tedious.  */
		size_t nAnimationIndex = 0;
	};
}
#endif // !ADV_H_
