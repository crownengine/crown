/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
	public enum LinkType
	{
		EVENT,
		VARIABLE
	}

	public struct Link
	{
		public LinkType _type;
		public Guid FromNode;
		public Guid ToNode;
		public int FromRow;
		public int ToRow;

		public Link (Guid from, int lrow, Guid to, int rrow)
		{
			FromNode = from;
			ToNode = to;
			FromRow = lrow;
			ToRow = rrow;
		}
	}

	public class GraphStore
	{
		// Data
		// private HashMap<Guid, Node> _nodes;
		// private HashMap<Guid, Link> _links;

		// Signals
		public signal void node_inserted(Guid id);
		public signal void node_deleted(Guid id);
		public signal void link_inserted(Guid id);
		public signal void link_deleted(Guid id);

		public GraphStore()
		{
			// _nodes = new HashMap<Guid, Node>();
			// _links = new HashMap<Guid, Link>();
		}
	}
}
