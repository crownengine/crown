/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
public class Unit
{
	public Database _db;
	public Guid _unit;
	public Database _prefabs;

	public Unit(Database db, Guid unit, Database? prefabs)
	{
		_db = db;
		_unit = unit;
		_prefabs = prefabs != null ? prefabs : new Database();
	}

	public Value? get_component_property(Guid component_id, string key)
	{
		// Search in components
		{
			Value? components = _db.get_property(_unit, "components");
			if (components != null && ((HashSet<Guid?>)components).contains(component_id))
				return _db.get_property(component_id, key);
		}

		// Search in modified components
		{
			Value? value = _db.get_property(_unit, "modified_components.#" + component_id.to_string() + "." + key);
			if (value != null)
				return value;
		}

		// Search in prefab's components
		{
			Value? value = _db.get_property(_unit, "prefab");
			if (value != null)
			{
				string prefab = (string)value;
				Value? pcvalue = _prefabs.get_property(GUID_ZERO, prefab + ".components");
				if (pcvalue != null)
				{
					HashSet<Guid?> prefab_components = (HashSet<Guid?>)pcvalue;
					if (prefab_components.contains(component_id))
						return _prefabs.get_property(component_id, key);
				}
			}
		}

		assert(false);
		return null;
	}

	public void set_component_property(Guid component_id, string key, Value? value)
	{
		// Search in components
		{
			Value? components = _db.get_property(_unit, "components");
			if (components != null && ((HashSet<Guid?>)components).contains(component_id))
			{
				_db.set_property(component_id, key, value);
				return;
			}
		}

		_db.set_property(_unit, "modified_components.#" + component_id.to_string() + "." + key, value);
	}

	public static bool has_component_static(Database db, Database prefabs_db, Guid unit_id, string component_type, ref Guid ref_component_id)
	{
		// Search in components
		{
			Value? value = db.get_property(unit_id, "components");
			if (value != null)
			{
				HashSet<Guid?> components = (HashSet<Guid?>)value;
				foreach (Guid component_id in components)
				{
					if((string)db.get_property(component_id, "type") == component_type)
					{
						ref_component_id = component_id;
						return true;
					}
				}
			}
		}

		{
			string[] keys = db.get_keys(unit_id);
			foreach (string m in keys)
			{
				if (!m.has_prefix("modified_components.#"))
					continue;

				// 0                   21                                   58  62
				// |                    |                                    |   |
				// modified_components.#f56420ad-7f9c-4cca-aca5-350f366e0dc0.type
				string id = m[21:57];
				string type_or_name = m[58:62];

				if (!type_or_name.has_prefix("type"))
					continue;

				if ((string)db.get_property(unit_id, m) == component_type)
				{
					ref_component_id = Guid.parse(id);
					return true;
				}
			}
		}

		{
			Value? value = db.get_property(unit_id, "prefab");
			if (value != null)
			{
				string prefab = (string)value;
				Value? pcvalue = prefabs_db.get_property(GUID_ZERO, prefab + ".components");
				if (pcvalue != null)
				{
					HashSet<Guid?> prefab_components = (HashSet<Guid?>)pcvalue;
					foreach (Guid component_id in prefab_components)
					{
						if((string)prefabs_db.get_property(component_id, "type") == component_type)
						{
							ref_component_id = component_id;
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	/// Returns whether the unit has the component_type.
	public bool has_component(string component_type, ref Guid ref_component_id)
	{
		return Unit.has_component_static(_db, _prefabs, _unit, component_type, ref ref_component_id);
	}

	public void remove_component(Guid component_id)
	{
		_db.remove_from_set(GUID_ZERO, "components", component_id);
	}

	/// Returns whether the unit has a prefab.
	public bool has_prefab()
	{
		return _db.get_property(_unit, "prefab") != null;
	}

	/// Returns wheter the unit is a light unit.
	public bool is_light()
	{
		return has_prefab()
			&& (string)_db.get_property(_unit, "prefab") == "core/units/light";
	}
}

}
