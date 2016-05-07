/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	namespace UnitPreviewAPI
	{
		const string[] _placeables =
		{
			"unknown",
			"core/units/sound",
		};

		public string set_preview_unit(PlaceableType placeable_type, string name)
		{
			return "UnitPreview:set_preview_unit(\"%s\")".printf(placeable_type == PlaceableType.Unit ? name : _placeables[(int)placeable_type]);
		}
	}
}
