#ifndef ImGuiEditor_hpp
#define ImGuiEditor_hpp

namespace ecs {
	class RenderSystem;
}

class ImGuiEditor {
public:
	ImGuiEditor();
	void render();
	void render_global_editor();
	void render_camera_editor();
	void render_entity_editor();

	bool pixel_style;
	bool stop_rotate;
	bool normal_debug;
	bool enable_reflection;
	float global_ambient_strength;
	int icosphere_accuracy;

private:
	ecs::RenderSystem* ref_render_system;
};

#endif