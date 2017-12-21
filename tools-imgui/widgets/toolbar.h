namespace ImGui
{
	bool BeginToolbar(const char* str_id, ImVec2& pos, ImVec2& size);
	void EndToolbar();
	bool ToolbarButton(ImTextureID texture, const ImVec4& bg_color, const char* tooltip);
}
