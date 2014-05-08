using System.Collections.Generic;

namespace UnitEditor
{
	//------------------------------------------------------------------------------
	public class Renderable
	{
		public string node;
		public string type;
		public string resource;
		public bool visible;
	}

	//------------------------------------------------------------------------------
	public class Node
	{
		public string parent;
		public float[] position = new float[3];
		public float[] rotation = new float[4];
	}

	//------------------------------------------------------------------------------
	public class Actor
	{
		public string name;
		public string node;
		public string actor_class;
		public float mass;
	}

	//------------------------------------------------------------------------------
	public class Shape
	{
		public string type;
		public string shape_class;
		public string material;
		public float data_0;
		public float data_1;
		public float data_2;
		public float data_3;
	}

	//------------------------------------------------------------------------------
	public class Joint
	{
		public string type;
		public string actor_0;
		public float[] anchor_0 = new float[3];
		public string actor_1;
		public float[] anchor_1 = new float[3];
	}

} // namespace UnitEditor