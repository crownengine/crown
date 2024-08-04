/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ThumbnailCache
{
	[Compact]
	public struct CacheEntry
	{
		// Pixbuf area inside the atlas.
		Gdk.Pixbuf? pixbuf;
		unowned List<StringId64?> lru;
	}

	public const int THUMBNAIL_SIZE = 64;

	public Project _project;
	public RuntimeInstance _thumbnail;
	public Gdk.Pixbuf _atlas;
	public GLib.List<StringId64?> _list;
	public Gee.HashMap<StringId64?, CacheEntry?> _map;
	public uint _max_cache_size;

	// Called when the cache changed its content.
	public signal void changed();

	public ThumbnailCache(Project project, RuntimeInstance thumbnail, uint max_cache_size)
	{
		_project = project;
		_thumbnail = thumbnail;
		_list = new GLib.List<StringId64?>();
		_map = new Gee.HashMap<StringId64?, CacheEntry?>(StringId64.hash_func, StringId64.equal_func);

		reset(max_cache_size);
	}

	public string thumbnail_path(string resource_path)
	{
		GLib.File file = GLib.File.new_for_path(_project.absolute_path(resource_path));
		uint8 uri_md5[16];

		Md5.State st = Md5.State();
		st.append(file.get_uri().data);
		st.finish(out uri_md5);

		string thumb_filename = "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x.png".printf(uri_md5[0]
			, uri_md5[1]
			, uri_md5[2]
			, uri_md5[3]
			, uri_md5[4]
			, uri_md5[5]
			, uri_md5[6]
			, uri_md5[7]
			, uri_md5[8]
			, uri_md5[9]
			, uri_md5[10]
			, uri_md5[11]
			, uri_md5[12]
			, uri_md5[13]
			, uri_md5[14]
			, uri_md5[15]
			);

		return GLib.Path.build_filename(_thumbnails_normal_dir.get_path(), thumb_filename);
	}

	public Gdk.Pixbuf? create_thumbnail_subpixbuf()
	{
		int thumbs_per_row = _atlas.get_width() / THUMBNAIL_SIZE;
		int thumb_row = (int)_list.length() / thumbs_per_row;
		int thumb_col = (int)_list.length() % thumbs_per_row;
		int dest_x = thumb_col * THUMBNAIL_SIZE;
		int dest_y = thumb_row * THUMBNAIL_SIZE;

		return new Gdk.Pixbuf.subpixbuf(_atlas
			, dest_x
			, dest_y
			, THUMBNAIL_SIZE
			, THUMBNAIL_SIZE
			);
	}

	public void thumbnail_ready(string type, string name, string thumb_path)
	{
		string resource_path = ResourceId.path(type, name);
		StringId64 resource_id = StringId64(resource_path);

		// Rename thumb_path to destination atomically.
		GLib.File thumb_path_tmp = GLib.File.new_for_path(thumb_path);
		GLib.File thumb_path_dst = GLib.File.new_for_path(thumbnail_path(resource_path));
		try {
			thumb_path_tmp.move(thumb_path_dst, GLib.FileCopyFlags.OVERWRITE);
		} catch (GLib.Error e) {
			loge(e.message);
		}

		CacheEntry? entry = _map.get(resource_id);
		if (entry == null)
			return;

		copy_thumbnail_from_path(entry.pixbuf, thumb_path_dst.get_path());
	}

	// Copies @a thumbnail inside the atlas at the position defined by @a subpixbuf.
	public void copy_thumbnail(Gdk.Pixbuf? subpixbuf, Gdk.Pixbuf? thumbnail)
	{
		thumbnail.copy_area(0
			, 0
			, THUMBNAIL_SIZE
			, THUMBNAIL_SIZE
			, subpixbuf
			, 0
			, 0
			);

		changed();
	}

	public void copy_thumbnail_from_path(Gdk.Pixbuf? subpixbuf, string thumbnail_path)
	{
		var thumbnail = new Gdk.Pixbuf.from_file_at_size(thumbnail_path
			, THUMBNAIL_SIZE
			, THUMBNAIL_SIZE
			);
		copy_thumbnail(subpixbuf, thumbnail);
	}

	public void reset(uint max_size)
	{
		_list = new GLib.List<StringId64?>(); // No clear?
		_map.clear();

		int height;
		int width = (int)Math.sqrt(max_size);
		width /= 4; // 4 bytes per pixel.
		width -= width % THUMBNAIL_SIZE;
		height = width;
		_max_cache_size = (width / THUMBNAIL_SIZE) * (height / THUMBNAIL_SIZE);

		_atlas = new Gdk.Pixbuf(Gdk.Colorspace.RGB
			, true
			, 8
			, width
			, height
			);
	}

	public Gdk.Pixbuf? get(string type, string name)
	{
		string resource_path = ResourceId.path(type, name);
		StringId64 resource_id = StringId64(resource_path);
		Gdk.Pixbuf? pixbuf = null;

		// Create a unique temporary file to store the thumbnail's data.
		GLib.File thumb_path_tmp = null;
		try {
			FileIOStream fs;
			thumb_path_tmp = GLib.File.new_tmp(null, out fs);
			fs.close();
		} catch (GLib.Error e) {
			loge(e.message);
		}

		if (!_map.has_key(resource_id)) {
			if (_list.length() == _max_cache_size) {
				// Evict the least recently used entry.
				unowned List<StringId64?> lru = _list.nth(0);
				// Reuse the subpixbuf from the evicted entry.
				pixbuf = _map.get(lru.data).pixbuf;
				_map.unset(lru.data);
				_list.remove_link(lru);
			}

			// Create a new subpixbuf if the entry has not been reused.
			if (pixbuf == null)
				pixbuf = create_thumbnail_subpixbuf();

			// Create a new cache entry.
			_list.append(resource_id);
			CacheEntry entry = { pixbuf, _list.last() };
			_map.set(resource_id, entry);

			GLib.File thumb_path_dst = GLib.File.new_for_path(thumbnail_path(resource_path));
			if (false && thumb_path_dst.query_exists()) {
				copy_thumbnail_from_path(pixbuf, thumb_path_dst.get_path());
			} else {
				// Append request to generate a thumbnail.
				_thumbnail.send_script(ThumbnailApi.add_request(type, name, thumb_path_tmp.get_path()));
				_thumbnail.send(DeviceApi.frame());
			}
		} else {
			CacheEntry? entry = _map.get(resource_id);
			pixbuf = entry.pixbuf;

			// Set resource_id as most recently used entry.
			_list.remove_link(entry.lru);

			_list.append(resource_id);
			entry.lru = _list.last();
			_map.set(resource_id, entry);
		}

		return pixbuf;
	}
}

} /* namespace Crown */
