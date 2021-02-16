/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
public class Unit
{
	public Database _db;
	public Guid _id;

	public Unit(Database db, Guid id)
	{
		_db = db;
		_id = id;
	}

	public Value? get_component_property(Guid component_id, string key)
	{
		Value? val;

		// Search in components
		val = _db.get_property(_id, "components");
		if (val != null)
		{
			if (((HashSet<Guid?>)val).contains(component_id))
				return _db.get_property(component_id, key);
		}

		// Search in modified_components
		val = _db.get_property(_id, "modified_components.#" + component_id.to_string() + "." + key);
		if (val != null)
			return val;

		// Search in prefab
		val = _db.get_property(_id, "prefab");
		if (val != null)
		{
			// Convert prefab path to object ID.
			string prefab = (string)val;
			Guid prefab_id = _db.get_property_guid(GUID_ZERO, prefab + ".unit");

			Unit unit = new Unit(_db, prefab_id);
			return unit.get_component_property(component_id, key);
		}

		return null;
	}

	public bool get_component_property_bool(Guid component_id, string key)
	{
		return (bool)get_component_property(component_id, key);
	}

	public double get_component_property_double(Guid component_id, string key)
	{
		return (double)get_component_property(component_id, key);
	}

	public string get_component_property_string(Guid component_id, string key)
	{
		return (string)get_component_property(component_id, key);
	}

	public Guid get_component_property_guid(Guid component_id, string key)
	{
		return (Guid)get_component_property(component_id, key);
	}

	public Vector3 get_component_property_vector3(Guid component_id, string key)
	{
		return (Vector3)get_component_property(component_id, key);
	}

	public Quaternion get_component_property_quaternion(Guid component_id, string key)
	{
		return (Quaternion)get_component_property(component_id, key);
	}

	public void set_component_property_bool(Guid component_id, string key, bool val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id))
		{
			_db.set_property_bool(component_id, key, val);
			return;
		}

		_db.set_property_bool(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_double(Guid component_id, string key, double val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id))
		{
			_db.set_property_double(component_id, key, val);
			return;
		}

		_db.set_property_double(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_string(Guid component_id, string key, string val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id))
		{
			_db.set_property_string(component_id, key, val);
			return;
		}

		_db.set_property_string(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_guid(Guid component_id, string key, Guid val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id))
		{
			_db.set_property_guid(component_id, key, val);
			return;
		}

		_db.set_property_guid(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_vector3(Guid component_id, string key, Vector3 val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id))
		{
			_db.set_property_vector3(component_id, key, val);
			return;
		}

		_db.set_property_vector3(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public void set_component_property_quaternion(Guid component_id, string key, Quaternion val)
	{
		// Search in components
		Value? components = _db.get_property(_id, "components");
		if (components != null && ((HashSet<Guid?>)components).contains(component_id))
		{
			_db.set_property_quaternion(component_id, key, val);
			return;
		}

		_db.set_property_quaternion(_id, "modified_components.#" + component_id.to_string() + "." + key, val);
	}

	public static bool has_component_static(out Guid component_id, string component_type, Database db, Guid unit_id)
	{
		Value? val;

		// Search in components
		val = db.get_property(unit_id, "components");
		if (val != null)
		{
			foreach (Guid id in (HashSet<Guid?>)val)
			{
				if((string)db.get_property(id, "type") == component_type)
				{
					component_id = id;
					return true;
				}
			}
		}

		// Search in modified_components
		string[] keys = db.get_keys(unit_id);
		foreach (string key in keys)
		{
			if (!key.has_prefix("modified_components.#"))
				continue;

			// 0                   21                                   58  62
			// |                    |                                    |   |
			// modified_components.#f56420ad-7f9c-4cca-aca5-350f366e0dc0.type
			int aa = 21;
			int bb = 57;
			int cc = 58;
			int dd = 62;
			string id = key[aa:bb];
			string suffix = key[cc:dd];

			if (!suffix.has_prefix("type"))
				continue;

			Value? type = db.get_property(unit_id, key);
			if (type != null && (string)type == component_type)
			{
				component_id = Guid.parse(id);
				return true;
			}
		}

		// Search in prefab
		val = db.get_property(unit_id, "prefab");
		if (val != null)
		{
			// Convert prefab path to object ID.
			string prefab = (string)val;
			Guid prefab_id = db.get_property_guid(GUID_ZERO, prefab + ".unit");

			return has_component_static(out component_id
				, component_type
				, db
				, prefab_id
				);
		}

		component_id = GUID_ZERO;
		return false;
	}

	/// Returns whether the unit has the component_type.
	public bool has_component(out Guid component_id, string component_type)
	{
		return Unit.has_component_static(out component_id, component_type, _db, _id);
	}

	public void remove_component(Guid component_id)
	{
		_db.remove_from_set(_id, "components", component_id);
	}

	/// Returns whether the unit has a prefab.
	public bool has_prefab()
	{
		return _db.has_property(_id, "prefab");
	}

	/// Returns whether the unit is a light unit.
	public bool is_light()
	{
		return has_prefab()
			&& _db.get_property_string(_id, "prefab") == "core/units/light";
	}

	/// Returns whether the unit is a camera unit.
	public bool is_camera()
	{
		return has_prefab()
			&& _db.get_property_string(_id, "prefab") == "core/units/camera";
	}
}

}
