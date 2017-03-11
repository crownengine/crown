/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

namespace Crown
{
	// public class TypeUtils
	// {
	// 	public static Cairo.Color HexToColor(long color)
	// 	{
	// 		return new Cairo.Color((float)((color >> 16) & 0xff) / 255.0, (float)((color >> 8) & 0xff) / 255.0, ((float)(color & 0xff)) / 255.0);
	// 	}

	// 	public struct TypeMap
	// 	{
	// 		public Type a;
	// 		public Type b;
	// 		public bool assignable;

	// 		public TypeMap(Type a, Type b, bool assign)
	// 		{
	// 			this.a = a;
	// 			this.b = b;
	// 			this.assignable = assign;
	// 		}
	// 	}

	// 	public static TypeMap[] TypeTable = new TypeMap[6]
	// 	{
	// 		new TypeMap(typeof(InEvent)           , typeof(OutEvent)           , true), // <- last 'true' redundant
	// 		new TypeMap(typeof(InVariableBool)    , typeof(OutVariableBool)    , true), // but could be used for something else
	// 		new TypeMap(typeof(InVariableFloat)   , typeof(OutVariableFloat)   , true), // so keep it as placeholder.
	// 		new TypeMap(typeof(InVariableVector3) , typeof(OutVariableVector3) , true),
	// 		new TypeMap(typeof(InVariableString)  , typeof(OutVariableString)  , true),
	// 		new TypeMap(typeof(InVariableUnit)    , typeof(OutVariableUnit)    , true)
	// 	}

	// 	public static Dictionary<Type, Cairo.Color> TypeColor = new Dictionary<Type, Color>
	// 	{
	// 		{ typeof(InEvent)            , HexToColor(0xe6e6e6) },
	// 		{ typeof(OutEvent)           , HexToColor(0xe6e6e6) },
	// 		{ typeof(InVariableBool)     , HexToColor(0x90ff29) },
	// 		{ typeof(InVariableFloat)    , HexToColor(0xf7924b) },
	// 		{ typeof(InVariableVector3)  , HexToColor(0x81dbed) },
	// 		{ typeof(InVariableString)   , HexToColor(0xdc7eff) },
	// 		{ typeof(InVariableUnit)     , HexToColor(0xfeff6b) },
	// 		{ typeof(OutVariableBool)    , HexToColor(0x90ff29) },
	// 		{ typeof(OutVariableFloat)   , HexToColor(0xf7924b) },
	// 		{ typeof(OutVariableVector3) , HexToColor(0x81dbed) },
	// 		{ typeof(OutVariableString)  , HexToColor(0xdc7eff) },
	// 		{ typeof(OutVariableUnit)    , HexToColor(0xfeff6b) },
	// 	}

	// 	public static Dictionary<string, Cairo.Color> NodeColor = new Dictionary<string, Color>
	// 	{
	// 		{ "Data",   HexToColor(0x4cab1a) },
	// 		{ "Event",  HexToColor(0xab1a1a) },
	// 		{ "Action", HexToColor(0x198eab) },
	// 		{ "Logic",  HexToColor(0x8e1aab) },
	// 	}
	// }
}
