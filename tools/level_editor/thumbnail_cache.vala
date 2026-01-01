/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class ThumbnailCache
{
	[Compact]
	public struct CacheEntry
	{
		int id;                        ///< Entry unique ID. Used to convert the entry to a Pixbuf area inside the atlas.
		unowned List<StringId64?> lru; ///< Pointer to LRU list entry.
		uint64 mtime;                  ///< Pixbuf last modification time.
		bool pending;                  ///< Whether a request to generate the thumbnail is pending.
	}

	public const int THUMBNAIL_SIZE = 64;

	public Project _project;
	public RuntimeInstance _thumbnail;
	public Gdk.Pixbuf _atlas;
	public int _mip0_width;
	public int _mip0_height;
	public GLib.List<StringId64?> _list;
	public Gee.HashMap<StringId64?, CacheEntry?> _map;
	public uint _max_cache_size;
	public bool _no_disk_cache; // Debug only: always go through server to get a thumbnail.
	public PixbufView _debug_pixbuf;
	public Gtk.Window _debug_window;

	// Called when the cache changed its content.
	public signal void changed();

	public ThumbnailCache(Project project, RuntimeInstance thumbnail, uint max_cache_size)
	{
		_project = project;
		_thumbnail = thumbnail;
		_list = new GLib.List<StringId64?>();
		_map = new Gee.HashMap<StringId64?, CacheEntry?>(StringId64.hash_func, StringId64.equal_func);
		_no_disk_cache = false;

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

	public Gdk.Pixbuf? thumbnail_subpixbuf(int entry_id, int thumb_size = THUMBNAIL_SIZE)
	{
		assert(thumb_size <= THUMBNAIL_SIZE);

		int thumbs_per_row = _mip0_width / THUMBNAIL_SIZE;
		int thumb_row = entry_id / thumbs_per_row;
		int thumb_col = entry_id % thumbs_per_row;
		int mip_level = THUMBNAIL_SIZE / thumb_size;
		int dest_x;
		int dest_y;

		if (mip_level == 1) {
			dest_x = thumb_col * thumb_size;
			dest_y = thumb_row * thumb_size;
		} else {
			int half_ml = mip_level / 2;
			dest_x = thumb_col * thumb_size + _mip0_width;
			dest_y = thumb_row * thumb_size + (int)(_mip0_height * ((half_ml - 1) / (double)half_ml));
		}

		return new Gdk.Pixbuf.subpixbuf(_atlas
			, dest_x
			, dest_y
			, thumb_size
			, thumb_size
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

		try {
			// Read thumbnail from disk.
			uint64 thumb_mtime = 0;
			GLib.FileInfo thumb_info = thumb_path_dst.query_info("*", GLib.FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
			GLib.DateTime? mdate = thumb_info.get_modification_date_time();
			if (mdate != null)
				thumb_mtime = mdate.to_unix() * 1000000000 + mdate.get_microsecond() * 1000; // Convert to ns.

			load_thumbnail_from_path(entry.id, thumb_path_dst.get_path());
			entry.mtime = thumb_mtime;
			entry.pending = false;
			_map.set(resource_id, entry);
			assert(_map.get(resource_id).mtime == thumb_mtime);
		} catch (GLib.Error e) {
			loge(e.message);
		}
	}

	// Copies @a thumbnail inside the atlas at the position defined by @a subpixbuf.
	public void copy_thumbnail(Gdk.Pixbuf? subpixbuf, Gdk.Pixbuf? thumbnail)
	{
		thumbnail.copy_area(0
			, 0
			, thumbnail.width
			, thumbnail.height
			, subpixbuf
			, 0
			, 0
			);

		changed();
	}

	// Generates mips for @a thumb_id. The main thumbnail for @a thumb_id
	// is assumed to be already loaded into the atlas.
	public void generate_mips(int thumb_id)
	{
		Gdk.Pixbuf main_thumbnail = thumbnail_subpixbuf(thumb_id, THUMBNAIL_SIZE);

		for (int size = THUMBNAIL_SIZE / 2; size >= 16; size /= 2) {
			Gdk.Pixbuf? mip = thumbnail_subpixbuf(thumb_id, size);
			Gdk.Pixbuf main_scaled = main_thumbnail.scale_simple(mip.width, mip.height, Gdk.InterpType.BILINEAR);
			copy_thumbnail(mip, main_scaled);
		}
	}

	public void load_thumbnail_from_path(int thumb_id, string thumbnail_path) throws GLib.Error
	{
		Gdk.Pixbuf? subpixbuf = thumbnail_subpixbuf(thumb_id, THUMBNAIL_SIZE);

		try {
			var thumbnail = new Gdk.Pixbuf.from_file_at_size(thumbnail_path
				, subpixbuf.width
				, subpixbuf.height
				);
			copy_thumbnail(subpixbuf, thumbnail);

			generate_mips(thumb_id);
		} finally {
			// Empty.
		}
	}

	public void reset(uint max_size)
	{
		_list = new GLib.List<StringId64?>(); // No clear?
		_map.clear();

		double mip0_max_area = (double)max_size * (2.0/3.0); // Remaining 1/3 for mip 1, 2, ...
		_mip0_width = (int)(Math.sqrt(mip0_max_area));
		_mip0_width /= 4; // 4 bytes per pixel.
		_mip0_width -= _mip0_width % THUMBNAIL_SIZE;
		_mip0_height = _mip0_width;
		_max_cache_size = (_mip0_width / THUMBNAIL_SIZE) * (_mip0_height / THUMBNAIL_SIZE);

		_atlas = new Gdk.Pixbuf(Gdk.Colorspace.RGB
			, true
			, 8
			, _mip0_width + _mip0_width / 2
			, _mip0_height
			);
		changed();
	}

	public Gdk.Pixbuf? get(string type, string name, int thumb_size = THUMBNAIL_SIZE)
	{
		if (!_project.is_loaded())
			return null;

		string resource_path = ResourceId.path(type, name);
		StringId64 resource_id = StringId64(resource_path);
		CacheEntry? entry = null;

		// Allocate a subpixbuf slot inside the atlas.
		if (_map.has_key(resource_id)) {
			entry = _map.get(resource_id);

			// Set resource_id as most recently used entry.
			_list.remove_link(entry.lru);

			_list.append(resource_id);
			entry.lru = _list.last();
			_map.set(resource_id, entry);
		} else {
			int entry_id = 0;

			if (_list.length() == _max_cache_size) {
				// Evict the least recently used entry.
				unowned List<StringId64?> lru = _list.nth(0);
				// Reuse the subpixbuf from the evicted entry.
				entry_id = _map.get(lru.data).id;
				_map.unset(lru.data);
				_list.remove_link(lru);
			} else {
				// Create a new subpixbuf if the entry is new.
				entry_id = (int)_list.length();
			}

			uint64 thumb_mtime = 0;
			try {
				if (!_no_disk_cache) {
					// Read thumbnail from disk.
					GLib.File thumb_path_dst = GLib.File.new_for_path(thumbnail_path(resource_path));
					GLib.FileInfo thumb_info = thumb_path_dst.query_info("*", GLib.FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
					GLib.DateTime? mdate = thumb_info.get_modification_date_time();
					if (mdate != null)
						thumb_mtime = mdate.to_unix() * 1000000000 + mdate.get_microsecond() * 1000; // Convert to ns.

					load_thumbnail_from_path(entry_id, thumb_path_dst.get_path());
				}
			} catch (GLib.Error e) {
				// Nobody cares.
			}

			// Create a new cache entry.
			_list.append(resource_id);
			entry = { entry_id, _list.last(), thumb_mtime, false };
			_map.set(resource_id, entry);
		}

		if (!entry.pending && (entry.mtime == 0 || entry.mtime <= _project.mtime(type, name))) {
			// On-disk thumbnail not found or outdated.
			// Ask the server to generate a fresh one if the data is ready.
			if (_project._data_compiled) {
				try {
					// Create a unique temporary file to store the thumbnail's data.
					FileIOStream fs;
					GLib.File thumb_path_tmp = GLib.File.new_tmp(null, out fs);
					fs.close();

					// Request a new thumbnail.
					entry.pending = true;
					_map.set(resource_id, entry);
					_thumbnail.send_script(ThumbnailApi.add_request(type, name, thumb_path_tmp.get_path()));
					_thumbnail.send(DeviceApi.frame());
				} catch (GLib.Error e) {
					loge(e.message);
				}
			}
		}

		return thumbnail_subpixbuf(entry.id, thumb_size);
	}

	public void show_debug_window(Gtk.Window? parent_window)
	{
		if (_debug_window == null) {
			_debug_pixbuf = new PixbufView();
			_debug_pixbuf._zoom = 0.4;
			_debug_window = new Gtk.Window();
			_debug_window.set_title("ThumbnailCache Debug");
			_debug_window.set_size_request(800, 800);
			_debug_window.add(_debug_pixbuf);
			this.changed.connect(() => {
					_debug_pixbuf.set_pixbuf(_atlas);
					_debug_pixbuf.queue_draw();
				});
		}

		_debug_window.set_transient_for(parent_window);
		_debug_window.show_all();
		_debug_window.present();
	}
}

} /* namespace Crown */
