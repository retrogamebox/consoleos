
// minigl is heavily based on the broadcom aqua kos

#include "minigl.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bcm_host.h"
#include "GLES/gl.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"

static void (*minigl_quit_function)(void) = (void (*)(void)) 0;
void minigl_quit(void) {
	printf("[MINIGL] Quitting ...\n");
	if (minigl_quit_function) minigl_quit_function();
}

static void minigl_catch_signal(int signal) {
	printf("[MINIGL] Got signal (%d)\n", signal);	
	minigl_quit();
	exit(1);
}

static EGLDisplay egl_display;
static EGLContext egl_context;
static EGLSurface egl_surface;

static EGL_DISPMANX_WINDOW_T egl_native_window;

void minigl_setup(void (*quit_function)(void)) {
	printf("[MINIGL] Setting up ...\n");
	minigl_quit_function = quit_function;
	
	// setup signals
	
	struct sigaction signal_interrupt_handler;
	signal_interrupt_handler.sa_handler = minigl_catch_signal;
	sigemptyset(&signal_interrupt_handler.sa_mask);
	sigaction(SIGINT, &signal_interrupt_handler, NULL);
	
	// setup egl context
	
	static const EGLint attribute_list[] = {
		EGL_RED_SIZE,   8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE,  8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE,
	};
	
	static const EGLint context_attributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 1, // using gles 1 so that we don't have to deal with shaders (possibly creating overhead we don't want)
		EGL_NONE,
	};
	
	bcm_host_init();
	
	egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(egl_display != EGL_NO_DISPLAY);
	assert(eglInitialize(egl_display, NULL, NULL) != EGL_FALSE);
	
	EGLConfig config;
	EGLint num_config;
	
	assert(eglSaneChooseConfigBRCM(egl_display, attribute_list, &config, 1, &num_config) != EGL_FALSE);
	assert(eglBindAPI(EGL_OPENGL_ES_API) != EGL_FALSE);
	
	egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, context_attributes);
	assert(egl_context != EGL_NO_CONTEXT);
	
	eglSwapInterval(egl_display, 1);
	
	const int VC_LCD = 0;
	assert(graphics_get_display_size(VC_LCD, (uint32_t*) &egl_native_window.width, (uint32_t*) &egl_native_window.height) >= 0);
	
	VC_RECT_T dst_rect = { .x = 0, .y = 0, .width = egl_native_window.width, .height = egl_native_window.height };
	VC_RECT_T src_rect = { .x = 0, .y = 0, .width = egl_native_window.width << 16, .height = egl_native_window.height << 16 };
	
	DISPMANX_DISPLAY_HANDLE_T dispman_display = vc_dispmanx_display_open(VC_LCD);
	DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
	
	egl_native_window.element = vc_dispmanx_element_add(dispman_update, dispman_display, 0 /* layer */, &dst_rect, 0 /* src */, &src_rect, DISPMANX_PROTECTION_NONE, 0 /* alpha */, 0 /* clamp */, 0 /* transform */);
	vc_dispmanx_update_submit_sync(dispman_update);
	
	egl_surface = eglCreateWindowSurface(egl_display, config, &egl_native_window, NULL);
	assert(egl_surface != EGL_NO_SURFACE);
	assert(eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context) != EGL_FALSE);
	
	// setup opengl
	
	glColorMask(1, 1, 1, 0);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	
	printf("[MINIGL] OpenGL info\n");
	printf("[MINIGL] \tVendor:   %s\n", glGetString(GL_VENDOR));
	printf("[MINIGL] \tRenderer: %s\n", glGetString(GL_RENDERER));
	printf("[MINIGL] \tVersion:  %s\n", glGetString(GL_VERSION));
}

uint32_t minigl_width (void) { return egl_native_window.width;  }
uint32_t minigl_height(void) { return egl_native_window.height; }

void minigl_flip(void) {
	eglSwapBuffers(egl_display, egl_surface);
}

void minigl_clear(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void minigl_viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
}

typedef struct {
	#define IPX_SIGNATURE 0x495058
	uint64_t signature;
	
	uint64_t version_major;
	uint64_t version_minor;
	
	uint64_t width;
	uint64_t height;
	uint64_t bpp;
} ipx_header_t;

minigl_surface_t* minigl_new_image_surface(const char* path) {
	minigl_surface_t* self = (minigl_surface_t*) malloc(sizeof(minigl_surface_t));
	
	// load ipx
	
	FILE* file = fopen(path, "rb");
	if (!file) {
		printf("[MINIGL] %s does not exist\n", path);
		return (minigl_surface_t*) 0;
	}
	
	ipx_header_t header;
	fread(&header, 1, sizeof(header), file);
	
	if (header.signature != IPX_SIGNATURE) {
		printf("[MINIGL] File is not IPX\n");
		fclose(file);
		return (minigl_surface_t*) 0;
	}
	
	uint32_t bytes = header.width * header.height * header.bpp / 8;
	char* final_data = (char*) malloc(bytes);
	
	fread(final_data, 1, bytes, file);
	fclose(file);
	
	// load texture
	
	self->width  = header.width;
	self->height = header.height;
	
	glGenTextures(1, &self->texture);
	glBindTexture(GL_TEXTURE_2D, self->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, final_data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	free(final_data);
	return self;
}

static const uint8_t minigl_indices[] = { 0, 1, 2, 0, 2, 3 };

static const float minigl_vertices[] = {
	-0.5,  0.5, 1.0,
	-0.5, -0.5, 1.0,
	 0.5, -0.5, 1.0,
	 0.5,  0.5, 1.0,
};

static const float minigl_coords[] = {
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,
};

void minigl_draw_surface(minigl_surface_t* self, float x, float y, float z, float width, float height, float r, float g, float b, float a) {
	float colours[4 * 4];
	
	for (int i = 0; i < 4; i++) {
		colours[i * 4 + 0] = r;
		colours[i * 4 + 1] = g;
		colours[i * 4 + 2] = b;
		colours[i * 4 + 3] = a;
	}
	
	float vertices[sizeof(minigl_vertices) / sizeof(*minigl_vertices)];
	for (int i = 0; i < 4; i++) {
		vertices[i * 3 + 2] = minigl_vertices[i * 3 + 2] * -z;
		
		vertices[i * 3 + 0] = minigl_vertices[i * 3 + 0] * width  + x;
		vertices[i * 3 + 1] = minigl_vertices[i * 3 + 1] * height + y;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	if (self->texture) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer (4, GL_FLOAT, 0, colours);
	
	if (self->texture) {
		glTexCoordPointer(2, GL_FLOAT, 0, minigl_coords);
	}
	
	glBindTexture(GL_TEXTURE_2D, self->texture);
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_BYTE, minigl_indices);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	if (self->texture) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void minigl_remove_surface(minigl_surface_t* self) {
	glDeleteTextures(1, &self->texture);
	free(self);
}
