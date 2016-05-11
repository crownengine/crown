/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

/*
 * Original C# code:
 * Public Domain Niklas Frykholm
 */

using Gee;
using GLib;

namespace Crown
{
	/// <summary>
	/// Provides functions for encoding and decoding files in the simplified JSON format.
	/// </summary>
	public class SJSON
	{
		/// <summary>
		///  Encodes the Hashtable t in the simplified JSON format. The Hashtable can
		///  contain, numbers, bools, strings, ArrayLists and Hashtables.
		/// </summary>
		public static string Encode(Hashtable t)
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
		public static Hashtable load(string path)
		{
			FileStream fs = FileStream.open(path, "r");
			// Get file size
			fs.seek (0, FileSeek.END);
			long size = fs.tell ();
			fs.rewind ();
			uint8[] bytes = new uint8[size];
			fs.read(bytes);
			return decode(bytes) as Hashtable;
		}

		/// <summary>
		/// Convenience function for saving a file.
		/// </summary>
		public static void save(Hashtable h, string path)
		{
			string s = Encode(h);
			FileStream fs = FileStream.open(path, "w");
			uint8[] bytes = s.data;
			fs.write(bytes);
		}

		static void write_root_object(Hashtable t, StringBuilder builder)
		{
		   write_object_fields(t, builder, 0);
		}

		static void write_object_fields(Hashtable t, StringBuilder builder, int indentation)
		{
			string[] keys = t.keys.to_array();
			// keys.sort(); // FIXME
			foreach (string key in keys) {
				write_new_line(builder, indentation);
				builder.append(key);
				builder.append(" = ");
				write(t[key], builder, indentation);
			}
		}

		static void write_new_line(StringBuilder builder, int indentation)
		{
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
				builder.append(((uint8)o).to_string());
			else if (o.holds(typeof(int)))
				builder.append(((int)o).to_string());
			else if (o.holds(typeof(float)))
				builder.append(((float)o).to_string());
			else if (o.holds(typeof(double)))
				builder.append(((double)o).to_string());
			else if (o.holds(typeof(string)))
				write_string((string)o, builder);
			else if (o.holds(typeof(ArrayList)))
				write_array((ArrayList)o, builder, indentation);
			else if (o.holds(typeof(Hashtable)))
				write_object((Hashtable)o, builder, indentation);
			else
				assert(false);
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
			builder.append_c('[');
			foreach (Value? item in a) {
				builder.append_c(' ');
				write(item, builder, indentation+1);
			}
			builder.append(" ]");
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
				assert(false);
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
			return (string)s.to_array();
		}

		static void consume(uint8 [] json, ref int index, string consume)
		{
			skip_whitespace(json, ref index);
			for (int i=0; i<consume.length; ++i) {
				if (json[index] != consume[i])
					assert(false);
				++index;
			}
		}

		static Value? parse_value(uint8 [] json, ref int index)
		{
			uint8 c = Next(json, ref index);

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
				assert(false);
				return null;
			}

		}

		static uint8 Next(uint8 [] json, ref int index)
		{
			skip_whitespace(json, ref index);
			return json[index];
		}

		static Hashtable parse_object(uint8 [] json, ref int index)
		{
			Hashtable ht = new Hashtable();
			consume(json, ref index, "{");
			skip_whitespace(json, ref index);

			while (Next(json, ref index) != '}') {
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
			while (Next(json, ref index) != ']') {
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
						assert(false);
					}
					else
					{
						assert(false);
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
