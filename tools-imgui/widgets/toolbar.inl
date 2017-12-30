namespace ImGui
{

bool BeginToolbar(const char* str_id, ImVec2& pos, ImVec2& size)
{
	bool is_global = GImGui->CurrentWindowStack.Size == 1;
	SetNextWindowPos(pos);
	ImVec2 frame_padding = GetStyle().FramePadding;
	PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	PushStyleVar(ImGuiStyleVar_WindowPadding, frame_padding);
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	float padding = frame_padding.y * 2;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
	| ImGuiWindowFlags_NoMove
	| ImGuiWindowFlags_NoResize
	| ImGuiWindowFlags_NoScrollbar
	| ImGuiWindowFlags_NoSavedSettings;

	if (size.x == 0)
		size.x = GetContentRegionAvailWidth();

	SetNextWindowSize(size);

	bool ret;
	if (is_global)
		ret = Begin(str_id, NULL, flags);
	else
		ret = BeginChild(str_id, size, false, flags);

	PopStyleVar(3);

	return ret;
}

void EndToolbar()
{
	ImVec2 frame_padding = ImGui::GetStyle().FramePadding;
	PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	PushStyleVar(ImGuiStyleVar_WindowPadding, frame_padding);
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

	ImVec2 pos = GetWindowPos();
	ImVec2 size = GetWindowSize();
	if (GImGui->CurrentWindowStack.Size == 2)
		End();
	else
		EndChild();

	PopStyleVar(3);
	ImGuiWindow* win = GetCurrentWindowRead();
	if (GImGui->CurrentWindowStack.Size > 1)
	{
		ImVec2 addend(0, size.y + GetStyle().FramePadding.y * 2);
		SetCursorScreenPos(ImVec2(pos.x + addend.x, pos.y + addend.y));
	}
}

bool ToolbarButton(ImTextureID texture, const ImVec4& bg_color, const char* tooltip)
{
	ImVec2 frame_padding = ImGui::GetStyle().FramePadding;
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, frame_padding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

	bool ret = false;
	ImGui::SameLine();
	ImVec4 tint_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
	if (ImGui::ImageButton(texture, ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), -1, bg_color, tint_color))
	{
		ret = true;
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%s", tooltip);
	}
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);
	return ret;
}


} // namespace ImGui
