#include <array>

#include "filter_feature.hpp"

namespace CRSLib::Can
{
	template<FilterWidth, FilterMode>
	struct Filter;

	template<>
	struct Filter<FilterWidth::bit32, FilterMode::mask> final
	{
		MaskedFrameFeature<FilterWidth::bit32> masked32;
	};

	template<>
	struct Filter<FilterWidth::bit32, FilterMode::list> final
	{
		std::array<FrameFeature<FilterWidth::bit32>, 2> feature32s;
	};

	template<>
	struct Filter<FilterWidth::bit16, FilterMode::mask> final
	{
		std::array<MaskedFrameFeature<FilterWidth::bit16>, 2> masked16s;
	};

	template<>
	struct Filter<FilterWidth::bit16, FilterMode::list> final
	{
		std::array<FrameFeature<FilterWidth::bit16>, 4> feature16s;
	};
}