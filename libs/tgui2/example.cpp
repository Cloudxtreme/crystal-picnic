#include "example.hpp"

int main(int argc, char **argv)
{
	// Initialize Allegro
	al_init();
	al_install_mouse();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();

	ALLEGRO_DISPLAY *display = al_create_display(800, 600);
	ALLEGRO_FONT *font = al_load_ttf_font("DejaVuSans.ttf", 10, 0);
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	ALLEGRO_TIMER *timer = al_create_timer(1.0/60.0);
	al_start_timer(timer);

	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_timer_event_source(timer));

	tgui::init(display);
	tgui::setFont(font);

	ExFrame *exFrame = new ExFrame(al_map_rgb(0x00, 0x00, 0xff));
	exFrame->setSize(200, 200);
	exFrame->setPosition(10, 10);

	ExButton *exButton = new ExButton("Exit", al_map_rgb(0x80, 0x80, 0x80));

	tgui::setNewWidgetParent(0);
	tgui::addWidget(exFrame);
	tgui::setNewWidgetParent(exFrame);
	tgui::addWidget(exButton);
	tgui::setFocus(exButton);
	// centers relative to parent (the frame)
	tgui::centerWidget(exButton, 100, 150);

	int ticks = 0;

	while (true) {
		ALLEGRO_EVENT event;
		do {
			al_wait_for_event(queue, &event);
			if (event.type == ALLEGRO_EVENT_TIMER) {
				ticks++;
			}
			else {
				tgui::handleEvent(&event);
			}
		} while (!al_event_queue_is_empty(queue));

		if (ticks > 10) ticks = 10;

		for (int i = 0; i < ticks; i++) {
			tgui::update();
		}

		if (ticks > 0) {
			al_clear_to_color(al_map_rgb(0x00, 0x00, 0x00));
			tgui::draw();
			al_flip_display();
		}

		ticks = 0;
	}
}

