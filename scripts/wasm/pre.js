/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

if (typeof window !== "undefined") {
	const args = [];
	for (const [key, value] of new URLSearchParams(window.location.search)) {
		args.push("--" + key);
		if (value)
			args.push(value);
	}
	Module["arguments"] = args.concat(Module["arguments"] || []);
}
