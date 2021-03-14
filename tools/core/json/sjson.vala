/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

/*
 * Original C# code:
 * Public Domain Niklas Frykholm
 */

using Gee;

namespace Crown
{
/// <summary>
/// Provides functions for encoding and decoding files in the simplified JSON format.
/// </summary>
[Compact]
public class SJSON
{
	/// <summary>
	///  Encodes the Hashtable t in the simplified JSON format. The Hashtable can
	///  contain, numbers, bools, strings, ArrayLists and Hashtables.
	/// </summary>
	public static string encode(Hashtable t)
	{
		StringBuilder sb = new StringBuilder();
		write_root_object(t, sb);
		sb.append_c('\n');
		return sb.str;
	}

	/// <summary>
	/// Encodes the object o in the simplified JSON format (not as a root object).
	/// </summary>
	public static string encode_object(Value? o)
	{
		StringBuilder sb = new StringBuilder();
		write(o, sb, 0);
		return sb.str;
	}

	/// <summary>
	/// Decodes a SJSON bytestream into a Hashtable with numbers, bools, strings,
	/// ArrayLists and Hashtables.
	/// </summary>
	public static Hashtable decode(uint8[] sjson)
	{
		int index = 0;
		return parse_root_object(sjson, ref index);
	}

	/// <summary>
	/// Convenience function for loading a file.
	/// </summary>
	public static Hashtable load_from_file(GLib.FileStream? fs)
	{
		if (fs == null)
			return new Hashtable();

		// Get file size
		fs.seek(0, FileSeek.END);
		size_t size = fs.tell();
		fs.rewind();
		if (size == 0)
			return new Hashtable();

		// Read whole file
		uint8[] bytes = new uint8[size];
		size_t bytes_read = fs.read(bytes);
		if (bytes_read != size)
			return new Hashtable();

		return decode(bytes) as Hashtable;
	}

	/// <summary>
	/// Convenience function for loading a file.
	/// </summary>
	public static Hashtable load_from_path(string path)
	{
		FileStream fs = FileStream.open(path, "rb");
		return load_from_file(fs);
	}

	/// <summary>
	/// Convenience function for saving a file.
	/// </summary>
	public static void save(Hashtable h, string path)
	{
		FileStream fs = FileStream.open(path, "wb");
		if (fs == null)
			return;

		fs.write(encode(h).data);
	}

	static void write_root_object(Hashtable t, StringBuilder builder)
	{
	   write_object_fields(t, builder, 0);
	}

	static void write_object_fields(Hashtable t, StringBuilder builder, int indentation)
	{
		ArrayList<string> keys = new ArrayList<string>.wrap(t.keys.to_array());
		keys.sort(Gee.Functions.get_compare_func_for(typeof(string)));
		foreach (string key in keys) {
			write_new_line(builder, indentation);
			builder.append(key);
			builder.append(" = ");
			write(t[key], builder, indentation);
		}
	}

	static void write_new_line(StringBuilder builder, int indentation)
	{
		if (builder.len > 0)
			builder.append_c('\n');
		for (int i = 0; i < indentation; ++i)
			builder.append_c('\t');
	}

	static void write(Value? o, StringBuilder builder, int indentation)
	{
		if (o == null)
			builder.append("null");
		else if (o.holds(typeof(bool)) && (bool)o == false)
			builder.append("false");
		else if (o.holds(typeof(bool)))
			builder.append("true");
		else if (o.holds(typeof(uint8)))
			builder.append_printf("%u", (uint8)o);
		else if (o.holds(typeof(int)))
			builder.append_printf("%d", (int)o);
		else if (o.holds(typeof(float)))
			builder.append_printf("%.9g", (float)o);
		else if (o.holds(typeof(double)))
			builder.append_printf("%.17g", (double)o);
		else if (o.holds(typeof(string)))
			write_string((string)o, builder);
		else if (o.holds(typeof(ArrayList)))
			write_array((ArrayList)o, builder, indentation);
		else if (o.holds(typeof(Hashtable)))
			write_object((Hashtable)o, builder, indentation);
		else
			GLib.assert(false);
	}

	static void write_string(string s, StringBuilder builder)
	{
		builder.append_c('"');
		for (int i=0; i<s.length; ++i) {
			char c = s[i];
			if (c == '"' || c == '\\')
				builder.append_c('\\');
			builder.append_c(c);
		}
		builder.append_c('"');
	}

	static void write_array(ArrayList<Value?> a, StringBuilder builder, int indentation)
	{
		ArrayList<Value?> a_sorted = a;
		a_sorted.sort((a, b) => {
			if (!a.holds(typeof(Hashtable)) || !b.holds(typeof(Hashtable)))
				return 0;

			Hashtable obj_a = a as Hashtable;
			Hashtable obj_b = b as Hashtable;
			if (!obj_a.has_key("id") || !obj_b.has_key("id"))
				return 0;

			Guid guid_a = Guid.parse(obj_a["id"] as string);
			Guid guid_b = Guid.parse(obj_b["id"] as string);
			return Guid.compare_func(guid_a, guid_b);
		});

		builder.append_c('[');
		foreach (Value? item in a_sorted) {
			write_new_line(builder, indentation+1);
			write(item, builder, indentation+1);
		}
		write_new_line(builder, indentation);
		builder.append_c(']');
	}

	static void write_object(Hashtable t, StringBuilder builder, int indentation)
	{
		builder.append_c('{');
		write_object_fields(t, builder, indentation+1);
		write_new_line(builder, indentation);
		builder.append_c('}');
	}

	static Hashtable parse_root_object(uint8 [] json, ref int index)
	{
		Hashtable ht = new Hashtable();
		while (!at_end(json, ref index)) {
			string key = parse_identifier(json, ref index);
			consume(json, ref index, "=");
			Value? value = parse_value(json, ref index);
			ht[key] = value;
		}
		return ht;
	}

	static bool at_end(uint8 [] json, ref int index)
	{
		skip_whitespace(json, ref index);
		return (index >= json.length);
	}

	static void skip_whitespace(uint8 [] json, ref int index)
	{
		while (index < json.length) {
			uint8 c = json[index];
			if (c == '/')
				skip_comment(json, ref index);
			else if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',')
				++index;
			else
				break;
		}
	}

	static void skip_comment(uint8 [] json, ref int index)
	{
		uint8 next = json[index + 1];
		if (next == '/')
		{
			while (index + 1 < json.length && json[index] != '\n')
				++index;
			++index;
		}
		else if (next == '*')
		{
			while (index + 2 < json.length && (json[index] != '*' || json[index + 1] != '/'))
				++index;
			index += 2;
		}
		else
			GLib.assert(false);
	}

	static string parse_identifier(uint8 [] json, ref int index)
	{
		skip_whitespace(json, ref index);

		if (json[index] == '"')
			return parse_string(json, ref index);

		ArrayList<uint8> s = new ArrayList<uint8>();
		while (true) {
			uint8 c = json[index];
			if (c == ' ' || c == '\t' || c == '\n' || c == '=')
				break;
			s.add(c);
			++index;
		}
		s.add('\0');
		return (string)s.to_array();
	}

	static void consume(uint8 [] json, ref int index, string consume)
	{
		skip_whitespace(json, ref index);
		for (int i=0; i<consume.length; ++i) {
			if (json[index] != consume[i])
				GLib.assert(false);
			++index;
		}
	}

	static Value? parse_value(uint8 [] json, ref int index)
	{
		uint8 c = next(json, ref index);

		if (c == '{')
			return parse_object(json, ref index);
		else if (c == '[')
			return parse_array(json, ref index);
		else if (c == '"')
			return parse_string(json, ref index);
		else if (c == '-' || c >= '0' && c <= '9')
			return parse_number(json, ref index);
		else if (c == 't')
		{
			consume(json, ref index, "true");
			return true;
		}
		else if (c == 'f')
		{
			consume(json, ref index, "false");
			return false;
		}
		else if (c == 'n')
		{
			consume(json, ref index, "null");
			return null;
		}
		else
		{
			GLib.assert(false);
			return null;
		}

	}

	static uint8 next(uint8 [] json, ref int index)
	{
		skip_whitespace(json, ref index);
		return json[index];
	}

	static Hashtable parse_object(uint8 [] json, ref int index)
	{
		Hashtable ht = new Hashtable();
		consume(json, ref index, "{");
		skip_whitespace(json, ref index);

		while (next(json, ref index) != '}') {
			string key = parse_identifier(json, ref index);
			consume(json, ref index, "=");
			Value? value = parse_value(json, ref index);
			ht[key] = value;
		}
		consume(json, ref index, "}");
		return ht;
	}

	static ArrayList<Value?> parse_array(uint8 [] json, ref int index)
	{
		ArrayList<Value?> a = new ArrayList<Value?>();
		consume(json, ref index, "[");
		while (next(json, ref index) != ']') {
			Value? value = parse_value(json, ref index);
			a.add(value);
		}
		consume(json, ref index, "]");
		return a;
	}

	static string parse_string(uint8[] json, ref int index)
	{
		ArrayList<uint8> s = new ArrayList<uint8>();

		consume(json, ref index, "\"");
		while (true) {
			uint8 c = json[index];
			++index;
			if (c == '"')
				break;
			else if (c != '\\')
				s.add(c);
			else {
				uint8 q = json[index];
				++index;
				if (q == '"' || q == '\\' || q == '/')
					s.add(q);
				else if (q == 'b') s.add('\b');
				else if (q == 'f') s.add('\f');
				else if (q == 'n') s.add('\n');
				else if (q == 'r') s.add('\r');
				else if (q == 't') s.add('\t');
				else if (q == 'u')
				{
					GLib.assert(false);
				}
				else
				{
					GLib.assert(false);
				}
			}
		}
		s.add('\0');
		return (string)s.to_array();
	}

	static double parse_number(uint8[] json, ref int index)
	{
		int end = index;
		while (end < json.length && "0123456789+-.eE".index_of_char((char)json[end]) != -1)
			++end;
		uint8[] num = json[index:end];
		num += '\0';
		index = end;
		return double.parse((string)num);
	}
}

}
