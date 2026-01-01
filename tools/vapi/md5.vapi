/*
 * Copyright (c) 2012-2026 Daniele Bartolini <dbartolini@crownengine.org>
 * SPDX-License-Identifier: MIT
 */

[CCode (cheader_filename = "md5.h", lower_case_cprefix = "md5_")]
namespace Md5
{
[Compact]
[CCode (cname = "md5_state_t", cprefix = "md5_")]
public struct State
{
	[CCode (cname = "md5_init")]
	public State();
	public void append(uint8[] data);
	public void finish(out uint8 digest[16]);
}

} /* namespace Md5 */
