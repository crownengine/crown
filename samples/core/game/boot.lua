-- Copyright (c) 2012-2023 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/game"
require "main"

if not init then
	init = GameBase.init
end

if not update then
	update = GameBase.update
end

if not render then
	render = GameBase.render
end

if not shutdown then
	shutdown = GameBase.shutdown
end
