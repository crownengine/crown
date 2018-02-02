#include "core/memory/temp_allocator.h"
#include "core/strings/string_stream.h"

namespace crown
{

static bool scroll_to_bottom = false;
static char input_text_buffer[1024] = "";

int console_inputtext_callback(ImGuiTextEditCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
	{
		Console* console = (Console*) data->UserData;
		const Vector<DynamicString>& history = console->_console_history;
		s32& history_pos = console->_history_pos;
		const s32 prev_history_pos = history_pos;

		if (ImGuiKey_UpArrow == data->EventKey)
		{
			if (history_pos == -1)
				history_pos = vector::size(history) - 1;
			else if (history_pos > 0)
				history_pos--;
		}
		else if (ImGuiKey_DownArrow == data->EventKey)
		{
			if (history_pos != -1)
				if (++history_pos >= vector::size(history))
					history_pos = -1;
		}

		if (prev_history_pos != history_pos)
		{
			s32 len = snprintf(data->Buf, (size_t) data->BufSize, "%s", (history_pos >= 0) ? history[history_pos].c_str() : "");
			data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = len;
			data->BufDirty = true;
		}
	}

	return 0;
}

void console_draw(Console& console)
{
	if (!console._open) return;

	if (ImGui::BeginDock("Console", &console._open, ImGuiWindowFlags_NoScrollbar))
	{
		TCPSocket& client = console._client;
		Vector<ConsoleLog>& items = console._console_items;
		Vector<DynamicString>& history = console._console_history;
		Vector<DynamicString>& commands = console._console_commands;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
		static ImGuiTextFilter filter;
		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::PopStyleVar();
		ImGui::Separator();

		ImGui::BeginChild("ScrollingRegion", ImVec2(0,-ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
		for (uint32_t i = 0; i < vector::size(items); i++)
		{
			ConsoleLog item = items[i];
			if (!filter.PassFilter(item._message.c_str()))
				continue;

			ImVec4 col = ImVec4(1.0f,1.0f,1.0f,1.0f);

			switch (item._severity)
			{
				case LogSeverity::LOG_INFO: col = ImColor(0.7f, 0.7f, 0.7f, 1.0f); break;
				case LogSeverity::LOG_WARN: col = ImColor(1.0f, 1.0f, 0.4f, 1.0f); break;
				case LogSeverity::LOG_ERROR: col = ImColor(1.0f, 0.4f, 0.4f, 1.0f); break;
				default: CE_FATAL("Unknown Severity"); break;
			}
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(item._message.c_str());
			ImGui::PopStyleColor();
		}

		if (scroll_to_bottom)
			ImGui::SetScrollHere();

		scroll_to_bottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		if (ImGui::InputText("Input"
		, input_text_buffer
		, IM_ARRAYSIZE(input_text_buffer)
		, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory
		, console_inputtext_callback
		, &console))
		{
			char* input_end = input_text_buffer + strlen(input_text_buffer);
			while (input_end > input_text_buffer && input_end[-1] == ' ')
			{
				input_end--;
			}

			*input_end = 0;

			if (input_text_buffer[0])
			{
				ConsoleLog log(LogSeverity::LOG_INFO, input_text_buffer);
				console_execute_command(console, log);
			}

			strcpy(input_text_buffer, "");
			console._history_pos = -1;

			ImGui::SetKeyboardFocusHere(-1);
			scroll_to_bottom = true;
		}
	}
	ImGui::EndDock();
}

void console_execute_command(Console& console, ConsoleLog& command_line)
{
	TCPSocket& client = console._client;
	Vector<ConsoleLog>& items = console._console_items;
	Vector<DynamicString>& history = console._console_history;
	Vector<DynamicString>& commands = console._console_commands;

	vector::push_back(items, command_line);
	vector::push_back(history, command_line._message);

	// Process command
	if (strcmp(command_line._message.c_str(), "clear") == 0)
	{
		vector::clear(items);
	}
	else if (strcmp(command_line._message.c_str(), "history") == 0)
	{
		int first = vector::size(history)-10;
		for (uint32_t i = first > 0 ? first : 0; i < vector::size(history); i++)
		{
			char buffer[1024] = "";
			sprintf(buffer, "%3d: %s\n", i, history[i].c_str());
			ConsoleLog log(LogSeverity::LOG_INFO, buffer);
			vector::push_back(items, log);
		}
	}
	else if (strcmp(command_line._message.c_str(), "help") == 0)
	{
		ConsoleLog log;
		log._severity = LogSeverity::LOG_INFO;
		log._message = "commands: ";
		vector::push_back(items, log);
		for (uint32_t i = 0; i < vector::size(commands); i++)
		{
			log._severity = LogSeverity::LOG_INFO;
			log._message = commands[i].c_str();
			vector::push_back(items, log);
		}
	}
	else
	{
		// Send command to engine
		TempAllocator1024 ta;
		StringStream out(ta);
		out << "{\"type\":\"script\",\"script\":\"";
		out << command_line._message.c_str();
		out << "\"}";

		const char* cmd = string_stream::c_str(out);
		const uint32_t size = strlen32(cmd);
		client.write(&size, sizeof(uint32_t));
		client.write(cmd, size);
	}

	scroll_to_bottom = true;
}

void console_scroll_to_bottom()
{
	scroll_to_bottom = true;
}

} // namespace crown
