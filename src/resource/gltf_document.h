/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/types.h"
#   include "core/math/types.h"
#   include "core/memory/types.h"
#   include "core/strings/dynamic_string.h"
#   include "resource/types.h"

struct cgltf_data;
struct cgltf_animation;
struct cgltf_material;
struct cgltf_node;
struct cgltf_scene;
struct cgltf_skin;

namespace crown
{
struct GLTFDocument
{
	cgltf_data *data;
	cgltf_skin *skin;
	Vector<DynamicString> node_names;
	Vector<DynamicString> material_names;
	Vector<DynamicString> skin_names;
	Vector<DynamicString> animation_names;
	Array<u32> skin_groups;
	Array<u32> skin_group_primaries;
	Array<Matrix4x4> skin_transforms;
	Vector<cgltf_node *> bones;
	HashMap<StringId32, u16> bone_ids;

	///
	explicit GLTFDocument(Allocator &a);

	///
	~GLTFDocument();
};

namespace gltf
{
	/// Parses @a path, loads all buffers through @a opts and validates all ranges.
	s32 parse(GLTFDocument &gltf, const char *path, CompileOptions &opts);

	/// Returns the selected default scene, or the first scene if no default is set.
	const cgltf_scene *scene(const GLTFDocument &gltf);

	/// Returns whether @a node belongs to the selected scene.
	bool node_in_scene(const GLTFDocument &gltf, const cgltf_node *node);

	/// Returns stable unique names used while compiling resources.
	const char *node_name(const GLTFDocument &gltf, const cgltf_node *node);
	const char *material_name(const GLTFDocument &gltf, const cgltf_material *material);
	const char *skin_name(const GLTFDocument &gltf, const cgltf_skin *skin);
	const char *animation_name(const GLTFDocument &gltf, const cgltf_animation *animation);

	/// Selects the skeleton group containing @a skin and rebuilds the bone map.
	s32 select_skin(GLTFDocument &gltf, const cgltf_skin *skin, CompileOptions &opts);

	/// Selects a skeleton group by stable skin name. An empty name selects the default group.
	s32 select_skin(GLTFDocument &gltf, const char *name, CompileOptions &opts);

	/// Returns the group containing @a skin.
	u32 skin_group(const GLTFDocument &gltf, const cgltf_skin *skin);

	/// Returns the geometry transform that normalizes @a skin to its group binding.
	const Matrix4x4 &skin_transform(const GLTFDocument &gltf, const cgltf_skin *skin);

	/// Returns the transform that places the selected skeleton in mesh model space.
	Matrix4x4 skeleton_root_transform(const GLTFDocument &gltf);

	/// Returns the bone ID for @a node, or UINT16_MAX if it is not a bone.
	u16 bone_id(const GLTFDocument &gltf, const cgltf_node *node);

	/// Returns the index of @a node in @a skin's joint array, or -1 if it is not a joint.
	s32 joint_index(const cgltf_skin &skin, const cgltf_node *node);

	/// Reads the inverse bind matrix at @a joint, or returns the identity when none is provided.
	s32 binding_matrix(Matrix4x4 &matrix
		, const cgltf_skin &skin
		, u32 joint
		, CompileOptions &opts
		);

	/// Converts a glTF vector from right-handed Y-up to right-handed Z-up.
	Vector3 vector3(const f32 *v);

	/// Converts a glTF quaternion from right-handed Y-up to right-handed Z-up.
	Quaternion quaternion(const f32 *q);

	/// Converts a column-major glTF matrix to right-handed Z-up.
	Matrix4x4 matrix4x4(const f32 *m);

	/// Returns the converted local transform of @a node.
	Matrix4x4 local_transform(const cgltf_node *node);

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
