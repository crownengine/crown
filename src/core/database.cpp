/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/strings/string.h"
#include "core/types.h"
#include "core/guid.h"
#include "core/memory/allocator.h"
#include "core/math/types.h"
#include "core/memory/memory.h"
#include "core/containers/hash_map.h"
#include "core/strings/string_stream.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "core/math/quaternion.h"
#include "core/math/vector3.h"
#include "core/json/types.h"
#include "core/json/sjson.h"
#include "core/json/json_object.h"
#include "core/containers/vector.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/file.h"
#include "core/containers/hash_set.h"

namespace crown
{
	struct ActionType
	{
		enum Enum
		{
			CREATE,
			DESTROY,
			SET_PROPERTY_NULL,
			SET_PROPERTY_BOOL,
			SET_PROPERTY_DOUBLE,
			SET_PROPERTY_STRING,
			SET_PROPERTY_GUID,
			SET_PROPERTY_VECTOR3,
			SET_PROPERTY_QUATERNION,
			ADD_TO_SET,
			REMOVE_FROM_SET,
			RESTORE_POINT
		};
	};

	struct ValueType
	{
		enum Enum
		{
			NIL,
			BOOL,
			DOUBLE,
			STRING,
			GUID,
			VECTOR3,
			QUATERNION,
			SET,
		};
	};

	struct Ledger
	{
		Allocator* _allocator;
		u8* _data;
		u32 _read;

		Ledger(Allocator& a)
			: _allocator(&a)
			, _data(NULL)
			, _read(0)
		{
			_data = (u8*)a.allocate(1024*1024);
			_read = 0;
		}

		~Ledger()
		{
			_allocator->deallocate(_data);
		}

		void clear()
		{
			_read = 0;
		}

		u32 size()
		{
			return _read;
		}

		void write(const void* data, u32 size)
		{
			memcpy(&_data[_read], data, size);
			_read += size;
		}

		template <typename T>
		void write(const T& data)
		{
			write(&data, sizeof(T));
		}

		void read(void* data, u32 size)
		{
			_read -= size;
			memcpy(data, &_data[_read], size);
		}

		template <typename T>
		void read(T& data)
		{
			read(&data, sizeof(T));
		}

		ValueType::Enum read_value_type()
		{
			_read -= (u32)sizeof(u32);
			u32 a = *(u32*)(&_data[_read]);
			return (ValueType::Enum)a;
		}

		void write_create_action(Guid id)
		{
			write(id);
			write(ActionType::CREATE);
		}

		void write_destroy_action(Guid id)
		{
			write(id);
			write(ActionType::DESTROY);
		}

		void write_set_property_null(Guid id, const char* key)
		{
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_NULL);
		}

		void write_set_property_bool(Guid id, const char* key, bool value)
		{
			write(value);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_BOOL);
		}

		void write_set_property_double(Guid id, const char* key, double value)
		{
			write(value);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_DOUBLE);
		}

		void write_set_property_string(Guid id, const char* key, const char* value)
		{
			const u32 value_len = strlen32(value) + 1;
			write(value, value_len);
			write(value_len);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_STRING);
		}

		void write_set_property_guid(Guid id, const char* key, const Guid& value)
		{
			write(value);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_GUID);
		}

		void write_set_property_vector3(Guid id, const char* key, const Vector3& value)
		{
			write(value);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_VECTOR3);
		}

		void write_set_property_quaternion(Guid id, const char* key, const Quaternion& value)
		{
			write(value);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::SET_PROPERTY_QUATERNION);
		}

		void write_add_to_set_action(Guid id, const char* key, Guid item_id)
		{
			write(item_id);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::ADD_TO_SET);
		}

		void write_remove_from_set_action(Guid id, const char* key, Guid item_id)
		{
			write(item_id);
			const u32 key_len = strlen32(key) + 1;
			write(key, key_len);
			write(key_len);
			write(id);
			write(ActionType::REMOVE_FROM_SET);
		}

		void write_restore_point(int id, u32 size, Guid* begin, Guid* end)
		{
			const u32 num_guids = end - begin;
			for (u32 i = 0; i < num_guids; ++i)
				write(begin[i]);
			write(num_guids);
			write(size);
			write(id);
			write(ActionType::RESTORE_POINT);
		}

		u32 peek_type()
		{
			return *(u32*)(&_data[_read - (u32)sizeof(u32)]);
		}
	};

	struct Value
	{
		Value()
			: allocator(NULL)
		{
		}

		~Value()
		{
			if (allocator != NULL)
				allocator->deallocate(value_string);
		}

		Value(const Value& other)
		{
			if (type == ValueType::STRING)
				allocator->deallocate(value_string);

			type = other.type;

			switch (type)
			{
			case ValueType::NIL:
				break;

			case ValueType::BOOL:
				value_bool = other.value_bool;
				break;

			case ValueType::DOUBLE:
				value_double = other.value_double;
				break;

			case ValueType::STRING:
				allocator = other.allocator;
				value_string = (char*)allocator->allocate(strlen32(other.value_string) + 1);
				strcpy(value_string, other.value_string);
				break;

			case ValueType::GUID:
				value_guid = other.value_guid;
				break;

			case ValueType::VECTOR3:
				value_vector3 = other.value_vector3;
				break;

			case ValueType::QUATERNION:
				value_quaternion = other.value_quaternion;
				break;

			default:
				break;
			}
		}

		Value& operator=(const Value& other)
		{
			if (type == ValueType::STRING)
				allocator->deallocate(value_string);

			type = other.type;

			switch (type)
			{
			case ValueType::NIL:
				break;

			case ValueType::BOOL:
				value_bool = other.value_bool;
				break;

			case ValueType::DOUBLE:
				value_double = other.value_double;
				break;

			case ValueType::STRING:
				allocator = other.allocator;
				value_string = (char*)allocator->allocate(strlen32(other.value_string) + 1);
				strcpy(value_string, other.value_string);
				break;

			case ValueType::GUID:
				value_guid = other.value_guid;
				break;

			case ValueType::VECTOR3:
				value_vector3 = other.value_vector3;
				break;

			case ValueType::QUATERNION:
				value_quaternion = other.value_quaternion;
				break;

			default:
				break;
			}

			return *this;
		}

		Allocator* allocator;
		ValueType::Enum type;
		union
		{
			bool value_bool;
			f64 value_double;
			char* value_string;
			Guid value_guid;
			Vector3 value_vector3;
			Quaternion value_quaternion;
		};
	};

	struct Object
	{
		ALLOCATOR_AWARE;

		HashMap<const char*, Value> _data;

		Object(Allocator& a)
			: _data(a)
		{
		}
	};

	template<>
	struct hash<Guid>
	{
		u32 operator()(const Guid val) const
		{
			return val.data1 ^ val.data2 ^ val.data3 ^ val.data4;
		}
	};

	template<>
	struct hash<const char*>
	{
		u32 operator()(const char* val) const
		{
			return murmur32(val, strlen32(val), 0);
		}
	};

	struct Database
	{
		Allocator* _allocator;
		HashMap<Guid, Object*> _objects;
		const HashMap<StringId32, const char*>& _key_database;

		Database(Allocator& a, const HashMap<StringId32, const char*>& key_database)
			: _allocator(&a)
			, _objects(a)
			, _key_database(key_database)
		{
			// This is a special field which stores all objects
			hash_map::set(_objects, GUID_ZERO, CE_NEW(*_allocator, Object)(*_allocator));
		}

		~Database()
		{
			auto cur = hash_map::begin(_objects);
			auto end = hash_map::end(_objects);
			for (; cur != end; ++cur)
			{
				HASH_MAP_SKIP_HOLE(_objects, cur);

				CE_DELETE(*_allocator, cur->second);
			}
		}

		void create_internal(Guid id)
		{
			Object* obj = CE_NEW(*_allocator, Object)(*_allocator);
			hash_map::set(_objects, id, obj);
		}

		void destroy_internal(Guid id)
		{
			Object* obj = hash_map::get(_objects, id, (Object*)NULL);
			CE_DELETE(*_allocator, obj);
		}

		void set_property_bool_internal(Guid guid, const char* key, bool value)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::BOOL;
			val.value_bool = value;
			hash_map::set(obj->_data, key, val);
		}

		void set_property_double_internal(Guid guid, const char* key, f64 value)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::DOUBLE;
			val.value_double = value;
			hash_map::set(obj->_data, key, val);
		}

		void set_property_string_internal(Guid guid, const char* key, const char* value)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::STRING;
			val.allocator = &default_allocator();
			val.value_string = (char*)val.allocator->allocate(strlen32(value) + 1);
			strcpy(val.value_string, value);
			hash_map::set(obj->_data, key, val);
		}

		void set_property_guid_internal(Guid guid, const char* key, Guid value)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::GUID;
			val.value_guid = value;
			hash_map::set(obj->_data, key, val);
		}

		void set_property_vector3_internal(Guid guid, const char* key, Vector3 value)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::VECTOR3;
			val.value_vector3 = value;
			hash_map::set(obj->_data, key, val);
		}

		void set_property_quaternion_internal(Guid guid, const char* key, Quaternion value)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::QUATERNION;
			val.value_quaternion = value;
			hash_map::set(obj->_data, key, val);
		}

		Vector3 get_property_vector3(Guid guid, const char* key)
		{
			Object* obj = hash_map::get(_objects, guid, (Object*)NULL);

			Value val;
			val.type = ValueType::NIL;
			val = hash_map::get(obj->_data, key, val);

			return val.value_vector3;
		}

		void load(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject object(ta);
			sjson::parse(json, object);
			decode(object);
		}

		void decode(JsonObject& object)
		{
			// FIXME: reset();
			decode_object(GUID_ZERO, "", object);
		}

		void decode_object(Guid id, const char* key, JsonObject& object)
		{
			auto cur = json_object::begin(object);
			auto end = json_object::end(object);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(object, cur);

				if (cur->first == "id")
					continue;

				if (cur->first._data[0] == '#')
					continue;

				const char* key_null = NULL;
				const char* key = hash_map::get(_key_database, StringId32(cur->first.data(), cur->first.length()), key_null);
				if (key == key_null)
				{
					printf("KEY NOT IN DATABASE: %.*s\n", cur->first.length(), cur->first.data());
					return;
				}
				const char* value = cur->second;

				// Decode value
				switch (sjson::type(cur->second))
				{
				case JsonValueType::NIL:
					break;

				case JsonValueType::BOOL:
					set_property_bool_internal(id, key, sjson::parse_bool(value));
					break;

				case JsonValueType::NUMBER:
					set_property_double_internal(id, key, sjson::parse_float(value));
					break;

				case JsonValueType::STRING:
					{
						TempAllocator256 ta;
						DynamicString str(ta);
						sjson::parse_string(value, str);
						set_property_string_internal(id, key, str.c_str());
					}
					break;

				case JsonValueType::ARRAY:
					{
						TempAllocator256 ta;
						JsonArray arr(ta);
						sjson::parse_array(value, arr);

						if (array::size(arr) == 3 && sjson::type(arr[0]) == JsonValueType::NUMBER)
						{
							set_property_vector3_internal(id, key, sjson::parse_vector3(value));
						}
						else if (array::size(arr) == 4 && sjson::type(arr[0]) == JsonValueType::NUMBER)
							set_property_quaternion_internal(id, key, sjson::parse_quaternion(value));
						else
							decode_set(id, key, arr);
					}
					break;

				case JsonValueType::OBJECT:
					{
						const char* k = key; // FIXME, define k
						TempAllocator256 ta;
						JsonObject sub_object(ta);
						sjson::parse(cur->second, sub_object);
						decode_object(id, k, sub_object);
					}
					break;

				default:
					CE_FATAL("Unknown key type");
					break;
				}
			}
		}

		void decode_set(Guid id, const char* key, const JsonArray& json)
		{
			// Set should be created even if it is empty.
			create_empty_set(id, key);

			for (u32 i = 0; i < array::size(json); ++i)
			{
				TempAllocator128 ta;
				JsonObject obj(ta);
				sjson::parse_object(json[i], obj);
				Guid item_id = sjson::parse_guid(obj["id"]);
				create_internal(item_id);
				decode_object(item_id, "", obj);
				// add_to_set_internal(id, key, item_id);
			}
		}

		void create_empty_set(Guid id, const char* key)
		{
		}

		void dump()
		{
			TempAllocator4096 ta;
			StringStream ss(ta);

			auto cur = hash_map::begin(_objects);
			auto end = hash_map::end(_objects);
			for (; cur != end; ++cur)
			{
				HASH_MAP_SKIP_HOLE(_objects, cur);

				{
					ss << "{\n";
					ss << "    id = ";
					{
						TempAllocator128 ta;
						DynamicString str(ta);
						str.from_guid(cur->first);
						ss << '"' << str.c_str() << '"';
					}
					ss << "\n";
					encode_object(ss, cur->second);
					ss << "}\n";
				}
			}

			printf("%s\n", string_stream::c_str(ss));
		}

		void encode_object(StringStream& ss, Object* object)
		{
			auto cur = hash_map::begin(object->_data);
			auto end = hash_map::end(object->_data);
			for (; cur != end; ++cur)
			{
				HASH_MAP_SKIP_HOLE(object->_data, cur);

				ss << "    ";
				ss << cur->first;
				ss << " = ";
				encode_value(ss, cur->second);
				ss << "\n";
			}
		}

		void encode_value(StringStream& ss, const Value& value)
		{
			switch (value.type)
			{
			case ValueType::NIL:
				ss << "null";
				break;

			case ValueType::BOOL:
				ss << (value.value_bool ? "true" : "false");
				break;

			case ValueType::DOUBLE:
				ss << value.value_double;
				break;

			case ValueType::STRING:
				ss << '"' << value.value_string << '"';
				break;

			case ValueType::GUID:
			{
				TempAllocator128 ta;
				DynamicString str(ta);
				str.from_guid(value.value_guid);
				ss << '"' << str.c_str() << '"';
			}
				break;

			case ValueType::VECTOR3:
				ss << "[ " << value.value_vector3.x << " " << value.value_vector3.y << " " << value.value_vector3.z << " ]";
				break;

			case ValueType::QUATERNION:
				ss << "[ " << value.value_quaternion.x << " " << value.value_quaternion.y << " " << value.value_quaternion.z << " " << value.value_quaternion.w << " ]";
				break;

			default:
				break;
			}
		}
	};

	template<>
	struct equal_to<const char*>
	{
		bool operator()(const char* a, const char* b) const
		{
			return strcmp(a, b) == 0;
		};
	};

	void test_database()
	{
		memory_globals::init();
		{
#define DATABASE_KEY_SOUNDS "sounds"
#define DATABASE_KEY_CLASS "class"
#define DATABASE_KEY_COLLISION_FILTER "collision_filter"
#define DATABASE_KEY_COMPONENTS "components"
#define DATABASE_KEY_DATA "data"
#define DATABASE_KEY_EDITOR "editor"
#define DATABASE_KEY_GEOMETRY_NAME "geometry_name"
#define DATABASE_KEY_ID "id"
#define DATABASE_KEY_MASS "mass"
#define DATABASE_KEY_MATERIAL "material"
#define DATABASE_KEY_MESH_RESOURCE "mesh_resource"
#define DATABASE_KEY_MODIFIED_COMPONENTS "modified_components"
#define DATABASE_KEY_NAME "name"
#define DATABASE_KEY_POSITION "position"
#define DATABASE_KEY_PREFAB "prefab"
#define DATABASE_KEY_ROTATION "rotation"
#define DATABASE_KEY_SCALE "scale"
#define DATABASE_KEY_SCENE "scene"
#define DATABASE_KEY_SHAPE "shape"
#define DATABASE_KEY_TYPE "type"
#define DATABASE_KEY_UNITS "units"
#define DATABASE_KEY_VISIBLE "visible"

			HashMap<StringId32, const char*> key_database(default_allocator());
			hash_map::set(key_database, StringId32(DATABASE_KEY_SOUNDS), (const char*)DATABASE_KEY_SOUNDS);
			hash_map::set(key_database, StringId32(DATABASE_KEY_CLASS), (const char*)DATABASE_KEY_CLASS);
			hash_map::set(key_database, StringId32(DATABASE_KEY_COLLISION_FILTER), (const char*)DATABASE_KEY_COLLISION_FILTER);
			hash_map::set(key_database, StringId32(DATABASE_KEY_COMPONENTS), (const char*)DATABASE_KEY_COMPONENTS);
			hash_map::set(key_database, StringId32(DATABASE_KEY_DATA), (const char*)DATABASE_KEY_DATA);
			hash_map::set(key_database, StringId32(DATABASE_KEY_EDITOR), (const char*)DATABASE_KEY_EDITOR);
			hash_map::set(key_database, StringId32(DATABASE_KEY_GEOMETRY_NAME), (const char*)DATABASE_KEY_GEOMETRY_NAME);
			hash_map::set(key_database, StringId32(DATABASE_KEY_ID), (const char*)DATABASE_KEY_ID);
			hash_map::set(key_database, StringId32(DATABASE_KEY_MASS), (const char*)DATABASE_KEY_MASS);
			hash_map::set(key_database, StringId32(DATABASE_KEY_MATERIAL), (const char*)DATABASE_KEY_MATERIAL);
			hash_map::set(key_database, StringId32(DATABASE_KEY_MESH_RESOURCE), (const char*)DATABASE_KEY_MESH_RESOURCE);
			hash_map::set(key_database, StringId32(DATABASE_KEY_MODIFIED_COMPONENTS), (const char*)DATABASE_KEY_MODIFIED_COMPONENTS);
			hash_map::set(key_database, StringId32(DATABASE_KEY_NAME), (const char*)DATABASE_KEY_NAME);
			hash_map::set(key_database, StringId32(DATABASE_KEY_POSITION), (const char*)DATABASE_KEY_POSITION);
			hash_map::set(key_database, StringId32(DATABASE_KEY_PREFAB), (const char*)DATABASE_KEY_PREFAB);
			hash_map::set(key_database, StringId32(DATABASE_KEY_ROTATION), (const char*)DATABASE_KEY_ROTATION);
			hash_map::set(key_database, StringId32(DATABASE_KEY_SCALE), (const char*)DATABASE_KEY_SCALE);
			hash_map::set(key_database, StringId32(DATABASE_KEY_SCENE), (const char*)DATABASE_KEY_SCENE);
			hash_map::set(key_database, StringId32(DATABASE_KEY_SHAPE), (const char*)DATABASE_KEY_SHAPE);
			hash_map::set(key_database, StringId32(DATABASE_KEY_TYPE), (const char*)DATABASE_KEY_TYPE);
			hash_map::set(key_database, StringId32(DATABASE_KEY_UNITS), (const char*)DATABASE_KEY_UNITS);
			hash_map::set(key_database, StringId32(DATABASE_KEY_VISIBLE), (const char*)DATABASE_KEY_VISIBLE);

			Ledger ledger(default_allocator());
#if 1
			char* level_json = NULL;
			FilesystemDisk disk(default_allocator());
			File* file = disk.open("/home/dani/git/crown/samples/01-physics/test.level", FileOpenMode::READ);
			if (file)
			{
				u32 size = file->size();
				level_json = (char*)default_allocator().allocate(size + 1);
				file->read(level_json, size);
				level_json[size] = '\0';
				disk.close(*file);
			}

			Database db(default_allocator(), key_database);
			db.load(level_json);
			// db.set_property_vector3_internal(guid::parse("f56420ad-7f9c-4cca-aca5-350f366e0dc0"), "position", vector3(1, 2, 3));
			// db.set_property_vector3_internal(guid::parse("f56420ad-7f9c-4cca-aca5-350f366e0dc0"), "position", vector3(4, 5, 6));
			// db.set_property_vector3_internal(guid::parse("f56420ad-7f9c-4cca-aca5-350f366e0dc0"), "position", vector3(7, 8, 9));
			db.dump();
			default_allocator().deallocate(level_json);
#else
			Database db(default_allocator(), key_database);
			Guid id = guid::new_guid();
			db.create_internal(id);
			db.set_property_vector3_internal(id, DATABASE_KEY_POSITION, vector3(1, 2, 3));
			db.set_property_vector3_internal(id, DATABASE_KEY_POSITION, vector3(4, 5, 6));
			db.set_property_quaternion_internal(id, DATABASE_KEY_ROTATION, quaternion(1, 2, 3, 0));
			db.set_property_string_internal(id, DATABASE_KEY_MASS, "apple");
			db.set_property_string_internal(id, DATABASE_KEY_MASS, "banana");
			db.dump();
			db.destroy_internal(id);
#endif
		}
		memory_globals::shutdown();
	}

} // namespace crown
