/*
* Copyright (c) 2012-2025 Daniele Bartolini et al.
* SPDX-License-Identifier: GPL-3.0-or-later
*/
namespace Crown {
    static Guid GUID_NONE_FOLDER  = { 0x0000000000000000u, 0x0000000000000000u };
    static Guid GUID_UNIT_FOLDER  = { 0x0000000000000000u, 0x0000000000000001u };
    static Guid GUID_SOUND_FOLDER = { 0x0000000000000000u, 0x0000000000000002u };

    struct RootFolderInfo {
        public Guid guid;
        public string name;
        public string object_type;
        public LevelTreeView.ItemType item_type;
        public string item_type_str;
        public LevelTreeView.ItemType contains_item_type;
        public string contains_item_type_str;
        public string contains_source_set_str;
    }

    static RootFolderInfo[] get_root_folder_info() {
        return new RootFolderInfo[] {
            { GUID_UNIT_FOLDER,  "Units",  OBJECT_TYPE_FOLDER_UNIT,  LevelTreeView.ItemType.FOLDER, "unit_folder", LevelTreeView.ItemType.UNIT, "unit", "units"},
            { GUID_SOUND_FOLDER, "Sounds", OBJECT_TYPE_FOLDER_SOUND, LevelTreeView.ItemType.FOLDER, "sound_folder", LevelTreeView.ItemType.SOUND, "sound", "sounds"}
        };
    }

    static Guid[] get_root_folder_guids() {
        return new Guid[] { GUID_UNIT_FOLDER, GUID_SOUND_FOLDER };
    }
} /* namespace Crown */
