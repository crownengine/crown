/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	namespace UnitPreviewApi
	{
		public string set_preview_resource(string placeable_type, string name)
		{
			return "UnitPreview:set_preview_resource(\"%s\", \"%s\")".printf(placeable_type, name);
		}
	}
}
