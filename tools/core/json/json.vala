/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Original C# code:
 * Public Domain Niklas Frykholm
 */

namespace Crown
{
public errordomain JsonSyntaxError
{
	BAD_TOKEN,
	BAD_VALUE,
	BAD_STRING,
	BAD_COMMENT
}

public errordomain JsonWriteError
{
	BAD_VALUE,
	FILE_OPEN,
	FILE_WRITE
}

/// <summary>
/// Provides functions for encoding and decoding files in the JSON format.
/// </summary>
[Compact]
public class JSON
{
	/// <summary>
	///  Encodes the hashtable t in the JSON format. The hash table can
	///  contain, numbers, bools, strings, GenericArray and HashTable.
	/// </summary>
	public static string encode(Value? t) throws JsonWriteError
	{
		StringBuilder sb = new StringBuilder();
		write(t, sb, 0);
		sb.append_c('\n');
		return sb.str;
	}

	/// <summary>
	/// Decodes a JSON bytestream into a hash table with numbers, bools, strings,
	/// GenericArray and HashTable.
	/// </summary>
	public static Value? decode(uint8[] sjson) throws JsonSyntaxError
	{
		int index = 0;
		return parse(sjson, ref index);
	}

	/// <summary>
	/// Convenience function for loading a file.
	/// </summary>
	public static GLib.HashTable<string, Value?> load(string path) throws JsonSyntaxError
	{
		FileStream fs = FileStream.open(path, "rb");
		if (fs == null)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		// Get file size
		fs.seek(0, FileSeek.END);
		size_t size = fs.tell();
		fs.rewind();
		if (size == 0)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		// Read whole file
		uint8[] bytes = new uint8[size];
		size_t bytes_read = fs.read(bytes);
		if (bytes_read != size)
			return new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);

		return (GLib.HashTable<string, Value?>)decode(bytes);
	}

	/// <summary>
	/// Convenience function for saving a file.
	/// </summary>
	public static void save(GLib.HashTable<string, Value?> h, string path) throws JsonWriteError
	{
		FileStream fs = FileStream.open(path, "wb");
		if (fs == null)
			throw new JsonWriteError.FILE_OPEN("Unable to open '%s'".printf(path));

		uint8[] data = encode(h).data;
		size_t len = data.length;
		if (fs.write(data) != len)
			throw new JsonWriteError.FILE_WRITE("Error while writing '%s'".printf(path));
	}

	static void write_new_line(StringBuilder builder, int indentation)
	{
		if (builder.len > 0)
			builder.append_c('\n');
		for (int i = 0; i < indentation; ++i)
			builder.append_c('\t');
	}

	static void write(Value? o, StringBuilder builder, int indentation) throws JsonWriteError
	{
		if (o == null)
			builder.append("null");
		else if (o.holds(typeof(bool)) && (bool)o == false)
			builder.append("false");
		else if (o.holds(typeof(bool)))
			builder.append("true");
		else if (o.holds(typeof(int)))
			builder.append_printf("%d", (int)o);
		else if (o.holds(typeof(float)))
			builder.append_printf("%.9g", (float)o);
		else if (o.holds(typeof(double)))
			builder.append_printf("%.17g", (double)o);
		else if (o.holds(typeof(string)))
			write_string((string)o, builder);
		else if (o.holds(typeof(GLib.GenericArray)))
			write_array((GLib.GenericArray<Value?>)o, builder, indentation);
		else if (o.holds(typeof(GLib.HashTable)))
			write_object((GLib.HashTable<string, Value?>)o, builder, indentation);
		else
			throw new JsonWriteError.BAD_VALUE("Unsupported value type '%s'".printf(o.type_name()));
	}

	static void write_string(string s, StringBuilder builder)
	{
		builder.append_c('"');
		for (int i = 0; i < s.length; ++i) {
			uint8 c = s[i];
			if (c == '"' || c == '\\') {
				builder.append_c('\\');
				builder.append_c((char)c);
			} else if (c == '\n') {
				builder.append_c('\\');
				builder.append_c('n');
			} else {
				builder.append_c((char)c);
			}
		}
		builder.append_c('"');
	}

	static void write_array(GLib.GenericArray<Value?> a, StringBuilder builder, int indentation) throws JsonWriteError
	{
		bool write_comma = false;
		builder.append("[ ");
		for (int i = 0; i < a.length; ++i) {
			if (write_comma)
				builder.append(", ");
			write(a[i], builder, indentation + 1);
			write_comma = true;
		}
		builder.append("]");
	}

	static void write_object(GLib.HashTable<string, Value?> t, StringBuilder builder, int indentation) throws JsonWriteError
	{
		builder.append_c('{');
		bool write_comma = false;
		GLib.HashTableIter<string, Value?> iter = GLib.HashTableIter<string, Value?>(t);
		unowned string key;
		unowned Value? value;
		while (iter.next(out key, out value)) {
			if (write_comma)
				builder.append(", ");
			write_new_line(builder, indentation);
			write(key, builder, indentation);
			builder.append(" : ");
			write(value, builder, indentation);
			write_comma = true;
		}
		write_new_line(builder, indentation);
		builder.append_c('}');
	}

	static void skip_whitespace(uint8[] json, ref int index)
	{
		while (index < json.length) {
			uint8 c = json[index];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',')
				++index;
			else
				break;
		}
	}

	static void consume(uint8[] json, ref int index, string consume) throws JsonSyntaxError
	{
		skip_whitespace(json, ref index);
		for (int i = 0; i < consume.length; ++i) {
			if (json[index] != consume[i])
				throw new JsonSyntaxError.BAD_TOKEN("Expected '%c' got '%c'".printf(consume[i], json[index]));
			++index;
		}
	}

	static Value? parse(uint8[] json, ref int index) throws JsonSyntaxError
	{
		uint8 c = next(json, ref index);

		if (c == '{') {
			return parse_object(json, ref index);
		} else if (c == '[') {
			return parse_array(json, ref index);
		} else if (c == '"') {
			return parse_string(json, ref index);
		} else if (c == '-' || c >= '0' && c <= '9') {
			return parse_number(json, ref index);
		} else if (c == 't') {
			consume(json, ref index, "true");
			return true;
		} else if (c == 'f') {
			consume(json, ref index, "false");
			return false;
		} else if (c == 'n') {
			consume(json, ref index, "null");
			return null;
		} else {
			throw new JsonSyntaxError.BAD_VALUE("Bad value");
		}
	}

	static uint8 next(uint8[] json, ref int index)
	{
		skip_whitespace(json, ref index);
		return json[index];
	}

	static GLib.HashTable<string, Value?> parse_object(uint8[] json, ref int index) throws JsonSyntaxError
	{
		GLib.HashTable<string, Value?> ht = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		consume(json, ref index, "{");
		skip_whitespace(json, ref index);

		while (next(json, ref index) != '}') {
			string key = parse_string(json, ref index);
			consume(json, ref index, ":");
			if (key.has_suffix("_binary"))
				ht[key] = (Value?)parse_binary(json, ref index);
			else
				ht[key] = parse(json, ref index);
		}
		consume(json, ref index, "}");
		return ht;
	}

	static GLib.GenericArray<Value?> parse_array(uint8[] json, ref int index) throws JsonSyntaxError
	{
		GLib.GenericArray<Value?> a = new GLib.GenericArray<Value?>();
		consume(json, ref index, "[");
		while (next(json, ref index) != ']') {
			Value? value = parse(json, ref index);
			a.add(value);
		}
		consume(json, ref index, "]");
		return a;
	}

	static uint8[] parse_binary(uint8[] json, ref int index) throws JsonSyntaxError
	{
		GLib.ByteArray s = new GLib.ByteArray();

		consume(json, ref index, "\"");
		while (true) {
			uint8 c = json[index];
			++index;
			if (c == '"') {
				break;
			} else if (c != '\\') {
				s.append({ c });
			} else {
				uint8 q = json[index];
				++index;
				if (q == '"' || q == '\\' || q == '/')
					s.append({ q });
				else if (q == 'b')
					s.append({ '\b' });
				else if (q == 'f')
					s.append({ '\f' });
				else if (q == 'n')
					s.append({ '\n' });
				else if (q == 'r')
					s.append({ '\r' });
				else if (q == 't')
					s.append({ '\t' });
				else if (q == 'u')
					throw new JsonSyntaxError.BAD_STRING("Unsupported escape character 'u'");
				else
					throw new JsonSyntaxError.BAD_STRING("Bad string");
			}
		}
		s.append({ '\0' });
		return s.steal();
	}

	static string parse_string(uint8[] json, ref int index) throws JsonSyntaxError
	{
		return (string)parse_binary(json, ref index);
	}

	static double parse_number(uint8[] json, ref int index)
	{
		int end = index;
		while ("0123456789+-.eE".index_of_char((char)json[end]) != -1)
			++end;
		uint8[] num = json[index : end];
		num += '\0';
		index = end;
		return double.parse((string)num);
	}
}

} /* namespace Crown */
