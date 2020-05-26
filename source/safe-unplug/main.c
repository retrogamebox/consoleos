
#include "minigl.h"
#include <math.h>

#define EXPECTED_FRAMERATE 60

static minigl_surface_t* box_surface;
static minigl_surface_t* cable_surface;
static minigl_surface_t* arrow_surface;

void quit(void) {
	minigl_remove_surface(box_surface);
	minigl_remove_surface(cable_surface);
	minigl_remove_surface(arrow_surface);
}

void main(void) {
	minigl_setup(quit);
	
	uint32_t width  = minigl_width ();
	uint32_t height = minigl_height();
	
	box_surface = minigl_new_image_surface("/kappa/services/safe-unplug/box.ipx");
	cable_surface = minigl_new_image_surface("/kappa/services/safe-unplug/cable.ipx");
	arrow_surface = minigl_new_image_surface("/kappa/services/safe-unplug/arrow.ipx");
	
	minigl_viewport(width / 2 - box_surface->width / 2, height / 2 - box_surface->height / 2, box_surface->width, box_surface->height);
	float cable_screen_height = (float) cable_surface->height / height * 4;
	
	float arrow_screen_width  = (float) arrow_surface->width  / width  * 4;
	float arrow_screen_height = (float) arrow_surface->height / height * 4;
	
	float animation = 0.0;
	
	float seconds = 0.0;
	while (1) {
		seconds += 1.0 / EXPECTED_FRAMERATE;
		minigl_clear(0.0, 0.0, 0.0, 1.0);
		
		int plugged = !((int) seconds % 3);
		
		if (plugged) animation = 0.0;
		else animation += (1.0 - animation) / 60;
		
		minigl_draw_surface(cable_surface, animation, 0.0, 0.0, 2.0, cable_screen_height, 1.0, 1.0, 1.0, 1.0);
		minigl_draw_surface(box_surface, 0.0, 0.0, 0.5, 2.0, 2.0, 1.0, 1.0, 1.0, 1.0);
		minigl_draw_surface(arrow_surface, -arrow_screen_width / 2 + animation, 0.5, 0.7, arrow_screen_width, arrow_screen_height, 1.0, 1.0, 1.0, -cos(!(((int) seconds + 2) % 3) * fmod(seconds, 2.0) * M_PI * 2) / 2 + 0.5);
		
		minigl_flip();
	}
	
	minigl_quit();
}
