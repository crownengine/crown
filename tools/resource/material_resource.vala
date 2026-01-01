/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct MaterialResource
{
	public Database _db;
	public Guid _id;

	public MaterialResource(Database db, Guid id, string shader)
	{
		_db = db;
		_id = id;

		_db.create(_id, OBJECT_TYPE_MATERIAL);
		_db.set_string(_id, "shader", shader);
	}

	public MaterialResource.mesh(Database db
		, Guid material_id
		, string? albedo_map = null
		, string? normal_map = null
		, string? metallic_map = null
		, string? roughness_map = null
		, string? ao_map = null
		, string? emission_map = null
		, Vector3 albedo = VECTOR3_ONE
		, double metallic = 0.0
		, double roughness = 1.0
		, Vector3 emission_color = VECTOR3_ZERO
		, double emission_intensity = 1.0
		, string? shader = null
		)
	{
		this(db, material_id, shader != null ? shader : "mesh");

		set_vector3("u_albedo", albedo);
		set_float("u_metallic", metallic);
		set_float("u_roughness", roughness);
		set_vector3("u_emission_color", emission_color);
		set_float("u_emission_intensity", emission_intensity);

		if (albedo_map != null) set_texture("u_albedo_map", albedo_map);
		if (normal_map != null) set_texture("u_normal_map", normal_map);
		if (metallic_map != null) set_texture("u_metallic_map", metallic_map);
		if (roughness_map != null) set_texture("u_roughness_map", roughness_map);
		if (ao_map != null) set_texture("u_ao_map", ao_map);
		if (emission_map != null) set_texture("u_emission_map", emission_map);

		set_float("u_use_albedo_map", (double)(albedo_map != null));
		set_float("u_use_normal_map", (double)(normal_map != null));
		set_float("u_use_metallic_map", (double)(metallic_map != null));
		set_float("u_use_roughness_map", (double)(roughness_map != null));
		set_float("u_use_ao_map", (double)(ao_map != null));
		set_float("u_use_emission_map", (double)(emission_map != null));
	}

	public MaterialResource.sprite(Database db
		, Guid material_id
		, string albedo_map
		, Quaternion color = Quaternion(1.0, 1.0, 1.0, 1.0)
		)
	{
		this(db, material_id, "sprite");

		set_texture("u_albedo_map", albedo_map);
		set_vector4("u_color", color);
	}

	public MaterialResource.gui(Database db, Guid material_id, string? albedo_map)
	{
		// FIXME: add defines list to ctor?
		string shader = "gui";
		if (albedo_map != null)
			shader += "+DIFFUSE_MAP";

		this(db, material_id, shader);

		if (albedo_map != null)
			set_texture("u_albedo_map", albedo_map);
	}

	public void set_float(string uniform_name, double value)
	{
		_db.set_string(_id
			, "uniforms.%s.type".printf(uniform_name)
			, "float"
			);
		_db.set_double(_id
			, "uniforms.%s.value".printf(uniform_name)
			, value
			);
	}

	public void set_vector2(string uniform_name, Vector2 value)
	{
		_db.set_string(_id
			, "uniforms.%s.type".printf(uniform_name)
			, "vector2"
			);
		_db.set_vector3(_id
			, "uniforms.%s.value".printf(uniform_name)
			, Vector3(value.x, value.y, 0.0)
			);
	}

	public void set_vector3(string uniform_name, Vector3 value)
	{
		_db.set_string(_id
			, "uniforms.%s.type".printf(uniform_name)
			, "vector3"
			);
		_db.set_vector3(_id
			, "uniforms.%s.value".printf(uniform_name)
			, value
			);
	}

	public void set_vector4(string uniform_name, Quaternion value)
	{
		_db.set_string(_id
			, "uniforms.%s.type".printf(uniform_name)
			, "vector4"
			);
		_db.set_quaternion(_id
			, "uniforms.%s.value".printf(uniform_name)
			, value
			);
	}

	public void set_texture(string sampler_name, string texture_name)
	{
		_db.set_string(_id, "textures.%s".printf(sampler_name), texture_name);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_MATERIAL, _id);
	}
}

} /* namespace Crown */
