/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct Version
{
	public int major;
	public int minor;
	public int micro;
	public int commit;

	public Version.parse(string s)
	{
		int p1 = s.index_of_char('.');
		int p2 = s.index_of_char('.', p1 + 1);
		int r  = s.index_of_char('-', p2 + 1);

		major = int.parse(s);
		minor = int.parse((string)((char*)s + p1 + 1));
		micro = int.parse((string)((char*)s + p2 + 1));
		commit = r < 0
			? -1
			: int.parse((string)((char*)s + r + 2))
			;
	}

	public bool newer_than(Version other)
	{
		if (major != other.major)
			return major > other.major;

		if (minor != other.minor)
			return minor > other.minor;

		if (micro != other.micro)
			return micro > other.micro;

		return commit > other.commit;
	}

	public static async bool query_remote(out Version remote)
	{
		remote = {};
		var file = File.new_for_uri(CROWN_VERSION_URL);

		try {
			uint8[] json;
			string? etag;

			yield file.load_contents_async(null, out json, out etag);

			// {"stable":{"crown":"0.64.6"},"unstable":{"crown-nightly":"0.65.0-r9727"}}
			GLib.HashTable<string, Value?> version = (GLib.HashTable<string, Value?>)JSON.decode(json);
			GLib.HashTable<string, Value?> stable = (GLib.HashTable<string, Value?>)version["stable"];
			// GLib.HashTable<string, Value?> unstable = (GLib.HashTable<string, Value?>)version["unstable"];
			string crown = (string)stable["crown"];

			remote = Version.parse(crown);
			return true;
		} catch (GLib.Error e) {
			// No-op.
		}

		return false;
	}
}

} /* namespace Crown */
