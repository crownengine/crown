/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	public class Project
	{
		// Data
		private string _source_dir;
		private string _toolchain_dir;
		private string _data_dir;
		private string _platform;

		private Database _files;

		public Project()
		{
			_source_dir = null;
			_toolchain_dir = null;
			_data_dir = null;
			_platform = "linux";

			_files = new Database();
		}

		public void load(string source_dir, string toolchain_dir, string data_dir)
		{
			_source_dir = source_dir;
			_toolchain_dir = toolchain_dir;
			_data_dir = data_dir;

			scan_source_dir();
		}

		public string source_dir()
		{
			return _source_dir;
		}

		public string toolchain_dir()
		{
			return _toolchain_dir;
		}

		public string data_dir()
		{
			return _data_dir;
		}

		public string platform()
		{
			return _platform;
		}

		public Database files()
		{
			return _files;
		}

		private void scan_source_dir()
		{
			list_directory_entries(File.new_for_path(_source_dir));
		}

		private void list_directory_entries(File dir, Cancellable? cancellable = null) throws Error
		{
			FileEnumerator enumerator = dir.enumerate_children(GLib.FileAttribute.STANDARD_NAME
				, FileQueryInfoFlags.NOFOLLOW_SYMLINKS
				, cancellable
				);

			FileInfo info = null;
			while (cancellable.is_cancelled () == false && ((info = enumerator.next_file (cancellable)) != null))
			{
				if (info.get_file_type () == FileType.DIRECTORY)
				{
					File subdir = dir.resolve_relative_path (info.get_name());
					list_directory_entries(subdir, cancellable);
				}
				else
				{
					string path = dir.get_path() + "/" + info.get_name();
					string path_rel = File.new_for_path(_source_dir).get_relative_path(File.new_for_path(path));
					string name = path_rel.substring(0, path_rel.last_index_of("."));
					string type = path_rel.substring(path_rel.last_index_of(".") + 1);

					Guid id = Guid.new_guid();
					_files.create(id);
					_files.set_property(id, "path", path);
					_files.set_property(id, "type", type);
					_files.set_property(id, "name", name);
					_files.add_to_set(GUID_ZERO, "data", id);
				}
			}

			if (cancellable.is_cancelled ())
			{
				throw new IOError.CANCELLED("Operation was cancelled");
			}
		}
	}
}
