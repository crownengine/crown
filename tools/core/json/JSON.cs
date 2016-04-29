/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

/*
 * Public Domain Niklas Frykholm
 */

using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System;

namespace Crown
{
	/// <summary>
	/// Provides functions for encoding and decoding files in the JSON format.
	/// </summary>
	public class JSON
	{
		/// <summary>
		///  Encodes the hashtable t in the JSON format. The hash table can
		///  contain, numbers, bools, strings, ArrayLists and Hashtables.
		/// </summary>
		public static string Encode(object t)
		{
			StringBuilder builder = new StringBuilder();
			Write(t, builder, 0);
			builder.AppendLine();
			return builder.ToString();
		}

		/// <summary>
		/// Decodes a JSON bytestream into a hash table with numbers, bools, strings,
		/// ArrayLists and Hashtables.
		/// </summary>
		public static object Decode(byte[] sjson)
		{
			int index = 0;
			return Parse(sjson, ref index);
		}

		/// <summary>
		/// Convenience function for loading a file.
		/// </summary>
		public static Hashtable Load(string path)
		{
			System.IO.FileStream fs = System.IO.File.Open(path, System.IO.FileMode.Open, System.IO.FileAccess.Read);
			byte[] bytes = new byte[fs.Length];
			fs.Read(bytes, 0, bytes.Length);
			fs.Close();
			return Decode(bytes) as Hashtable;
		}

		/// <summary>
		/// Convenience function for saving a file.
		/// </summary>
		public static void Save(Hashtable h, string path)
		{
			string s = Encode(h);
			System.IO.FileStream fs = System.IO.File.Open(path, System.IO.FileMode.Create);
			byte[] bytes = System.Text.Encoding.UTF8.GetBytes(s);
			fs.Write(bytes, 0, bytes.Count());
			fs.Close();
		}

		static void WriteNewLine(StringBuilder builder, int indentation)
		{
			builder.Append('\n');
			for (int i = 0; i < indentation; ++i)
				builder.Append('\t');
		}

		static void Write(object o, StringBuilder builder, int indentation)
		{
			if (o == null)
				builder.Append("null");
			else if (o is Boolean && (bool)o == false)
				builder.Append("false");
			else if (o is Boolean)
				builder.Append("true");
			else if (o is int)
				builder.Append((int)o);
			else if (o is float)
				builder.Append((float)o);
			else if (o is double)
				builder.Append((double)o);
			else if (o is string)
				WriteString((String)o, builder);
			else if (o is ArrayList)
				WriteArray((ArrayList)o, builder, indentation);
			else if (o is Hashtable)
				WriteObject((Hashtable)o, builder, indentation);
			else
				throw new ArgumentException("Unknown object");
		}

		static void WriteString(String s, StringBuilder builder)
		{
			builder.Append('"');
			for (int i = 0; i < s.Length; ++i)
			{
				Char c = s[i];
				if (c == '"' || c == '\\')
				{
					builder.Append('\\');
					builder.Append(c);
				}
				else if (c == '\n')
				{
					builder.Append('\\');
					builder.Append('n');
				}
				else
					builder.Append(c);
			}
			builder.Append('"');
		}

		static void WriteArray(ArrayList a, StringBuilder builder, int indentation)
		{
			bool write_comma = false;
			builder.Append("[ ");
			foreach (object item in a)
			{
				if (write_comma)
					builder.Append(", ");
				Write(item, builder, indentation + 1);
				write_comma = true;
			}
			builder.Append("]");
		}

		static void WriteObject(Hashtable t, StringBuilder builder, int indentation)
		{
			builder.Append('{');
			bool write_comma = false;
			foreach (DictionaryEntry de in t)
			{
				if (write_comma)
					builder.Append(", ");
				WriteNewLine(builder, indentation);
				Write(de.Key, builder, indentation);
				builder.Append(" : ");
				Write(de.Value, builder, indentation);
				write_comma = true;

			}
			WriteNewLine(builder, indentation);
			builder.Append('}');
		}

		static bool AtEnd(byte[] json, ref int index)
		{
			SkipWhitespace(json, ref index);
			return (index >= json.Length);
		}

		static void SkipWhitespace(byte[] json, ref int index)
		{
			while (index < json.Length)
			{
				byte c = json[index];
				if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',')
					++index;
				else
					break;
			}
		}

		static void Consume(byte[] json, ref int index, String consume)
		{
			SkipWhitespace(json, ref index);
			for (int i = 0; i < consume.Length; ++i)
			{
				if (json[index] != consume[i])
					throw new FormatException();
				++index;
			}
		}

		static object Parse(byte[] json, ref int index)
		{
			byte c = Next(json, ref index);

			if (c == '{')
				return ParseObject(json, ref index);
			else if (c == '[')
				return ParseArray(json, ref index);
			else if (c == '"')
				return ParseString(json, ref index);
			else if (c == '-' || c >= '0' && c <= '9')
				return ParseNumber(json, ref index);
			else if (c == 't')
			{
				Consume(json, ref index, "true");
				return true;
			}
			else if (c == 'f')
			{
				Consume(json, ref index, "false");
				return false;
			}
			else if (c == 'n')
			{
				Consume(json, ref index, "null");
				return null;
			}
			else
				throw new FormatException();
		}

		static byte Next(byte[] json, ref int index)
		{
			SkipWhitespace(json, ref index);
			return json[index];
		}

		static Hashtable ParseObject(byte[] json, ref int index)
		{
			Hashtable ht = new Hashtable();
			Consume(json, ref index, "{");
			SkipWhitespace(json, ref index);

			while (Next(json, ref index) != '}')
			{
				String key = ParseString(json, ref index);
				Consume(json, ref index, ":");
				if (key.EndsWith("_binary"))
					ht[key] = ParseBinary(json, ref index);
				else
					ht[key] = Parse(json, ref index);
			}
			Consume(json, ref index, "}");
			return ht;
		}

		static ArrayList ParseArray(byte[] json, ref int index)
		{
			ArrayList a = new ArrayList();
			Consume(json, ref index, "[");
			while (Next(json, ref index) != ']')
			{
				object value = Parse(json, ref index);
				a.Add(value);
			}
			Consume(json, ref index, "]");
			return a;
		}

		static byte[] ParseBinary(byte[] json, ref int index)
		{
			List<byte> s = new List<byte>();

			Consume(json, ref index, "\"");
			while (true)
			{
				byte c = json[index];
				++index;
				if (c == '"')
					break;
				else if (c != '\\')
					s.Add(c);
				else
				{
					byte q = json[index];
					++index;
					if (q == '"' || q == '\\' || q == '/')
						s.Add(q);
					else if (q == 'b') s.Add((byte)'\b');
					else if (q == 'f') s.Add((byte)'\f');
					else if (q == 'n') s.Add((byte)'\n');
					else if (q == 'r') s.Add((byte)'\r');
					else if (q == 't') s.Add((byte)'\t');
					else if (q == 'u')
					{
						throw new FormatException();
					}
					else
					{
						throw new FormatException();
					}
				}
			}
			return s.ToArray();
		}

		static String ParseString(byte[] json, ref int index)
		{
			return new UTF8Encoding().GetString(ParseBinary(json, ref index));
		}

		static Double ParseNumber(byte[] json, ref int index)
		{
			int end = index;
			while ("0123456789+-.eE".IndexOf((char)json[end]) != -1)
				++end;
			byte[] num = new byte[end - index];
			Array.Copy(json, index, num, 0, num.Length);
			index = end;
			String numstr = new UTF8Encoding().GetString(num);
			return Double.Parse(numstr);
		}
	}
}
