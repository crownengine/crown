using System.Collections.Generic;

namespace UnitEditor
{
	
//------------------------------------------------------------------------------
public class Renderable
{
	public Renderable(string node, string type, string resource, bool visible)
	{
		this.node = node;
		this.type = type;
		this.resource = resource;
		this.visible = visible;
	}

	public string node { set; get; }
	public string type { set; get; }
	public string resource { set; get; }
	public bool visible { set; get; }

	public override string ToString()
	{
		return "node: " + node + ", type: " + type + ", resource: " + resource + ", visible: " + visible + "\n";
	}
}

//------------------------------------------------------------------------------
public class Node
{
	public string parent { set; get; }
	public float[] position = new float[3];
	public float[] rotation = new float[4];
}

} // namespace UnitEditor