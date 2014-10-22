using System;

namespace Crown
{
namespace Console
{
	public class EntryHistory
	{
		private uint MaxRecords = 0;
		private uint Size = 0;
		private uint Current = 0;
		private string[] History = null;

		// Creates a new hisory with room for max_records records.
		public EntryHistory(uint max_records)
		{
			MaxRecords = max_records;
			History = new string[max_records];
		}

		// Push a new string into the history.
		public void Push(string text)
		{
			// Add command to history
			History[Size] = text;
			Size = Math.Min(Size + 1, MaxRecords - 1);
			Current = Size;
		}

		// Returns the previous entry in the history.
		public string Previous()
		{
			Current = Current > 0 ? Current -1 : 0;
			return History[Current];
		}

		// Returns the next entry in the history.
		public string Next()
		{
			Current = Math.Min(Current + 1, Size - 1);
			return History[Current];
		}
	}
} // namespace Console
} // namespace Crown
