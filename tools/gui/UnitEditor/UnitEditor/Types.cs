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
}

} // namespace UnitEditor
