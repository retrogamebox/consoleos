#include <stdint.h>

typedef struct {
	uint32_t width, height;
	uint32_t texture;
} minigl_surface_t;

void minigl_quit(void);
void minigl_setup(void (*quit_function)(void));

uint32_t minigl_width(void);
uint32_t minigl_height(void);

void minigl_viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

void minigl_flip(void);
void minigl_clear(float r, float g, float b, float a);

minigl_surface_t* minigl_new_image_surface(const char* path);
void minigl_draw_surface(minigl_surface_t* self, float x, float y, float z, float width, float height, float r, float g, float b, float a);
void minigl_remove_surface(minigl_surface_t* self);
