#include "core/memory/temp_allocator.h"
#include "core/strings/string_stream.h"

namespace ImGui
{

using namespace crown;

static bool scroll_to_bottom = false;

void console_draw(TCPSocket& client
	, Vector<ConsoleLog>& items
	, Vector<DynamicString>& history
	, Vector<DynamicString>& commands
	)
{
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

	// Command-line
	char buffer[1024] = "";
	if (ImGui::InputText("Input", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		char* input_end = buffer + strlen(buffer);
		while (input_end > buffer && input_end[-1] == ' ')
		{
			input_end--;
		}

		*input_end = 0;

		if (buffer[0])
		{
			ConsoleLog log(LogSeverity::LOG_INFO, buffer);
			console_execute_command(client, items, history, commands, log);
		}

		strcpy(buffer, "");
	}

	// Demonstrate keeping auto focus on the input box
	if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
}

void console_execute_command(TCPSocket& client
	, Vector<ConsoleLog>& items
	, Vector<DynamicString>& history
	, Vector<DynamicString>& commands
	, ConsoleLog& command_line
	)
{
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
	// else if (strcmp(command_line, "help") == 0)
	// {
	//  add_log(items, "commands:");
	//  for (uint32_t i = 0; i < vector::size(commands); i++)
	//      add_log(items, "- %s", commands[i].c_str());
	// }
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

} // namespace ImGui
