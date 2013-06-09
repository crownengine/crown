#include <gtkmm.h>
#include "CrownDrawingArea.h"
#include "Crown.h"

using namespace crown;

int main(int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);

	Device* engine = device();

	CrownDrawingArea crown_area(engine);
	crown_area.set_size_request(800, 500);

	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("ui/world-editor.glade");

	Gtk::Window* window = NULL;
	builder->get_widget("window1", window);

	window->set_title("World editor");

	Gtk::Box* box = NULL;
	builder->get_widget("box1", box);

	box->pack_start(crown_area);

	crown_area.show();

	Gtk::Main::run(*window);

	engine->shutdown();

	return EXIT_SUCCESS;
}
