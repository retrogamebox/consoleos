
#include "minigl.h"
#include <math.h>
#include <clock.h>
#include <unistd.h>

#define CONSOLEOS_INSTALL_SCRIPT "~/install.sh"

#define KAPPA_LAYER_DEVICE "/dev/mmcblk0p2"
#define SIGMA_LAYER_DEVICE "/dev/mmcblk0p5"
#define ALPHA_LAYER_DEVICE "/dev/mmcblk0p7"

#define DELTA_GOVENOR_SIGMA_LOG_PATH "/sigma/delta_govenor.slog"
#define WPA_OMEGA_CONFIG_PATH "/omega/wpa.ocon"

static void boot_process(void) {
	printf("[DELTA_GOVENOR] Checking for the " CONSOLEOS_INSTALL_SCRIPT " file ...\n");

	if (access(CONSOLEOS_INSTALL_SCRIPT, F_OK) != -1) {
		printf("[DELTA_GOVENOR] consoleOS install script exists, executing it ...\n");
		system("sh " CONSOLEOS_INSTALL_SCRIPT);
	}

	printf("[DELTA_GOVENOR] Checking integrity of Sigma layer (" SIGMA_LAYER_DEVICE ") ...\n");
	system("sudo fsck " SIGMA_LAYER_DEVICE);
	
	printf("[DELTA_GOVENOR] Mounting Sigma layer ...\n");
	system("sudo mount " SIGMA_LAYER_DEVICE " /sigma");
	
	printf("[DELTA_GOVENOR] Opening Delta Govenor Sigma log file (" DELTA_GOVENOR_SIGMA_LOG_PATH ") ...\n");
	if (!freopen(DELTA_GOVENOR_SIGMA_LOG_PATH, "w", stdout)) { // redirect stdout to log file
		printf("[DELTA_GOVENOR] Something has gone wrong, and the Sigma log file could not be opened\n[DELTA_GOVENOR] Aborting all operations and displaying fatal error message\n");
		
		printf("[DELTA_GOVENOR] Unmounting Kappa layer ... \n");
		system("sudo umount " KAPPA_LAYER_DEVICE);
		
		printf("[DELTA_GOVENOR] Unmounting Sigma layer ...\n");
		system("sudo umount " SIGMA_LAYER_DEVICE);
		
		while (1) {
			/// TODO display fatal error message
		}
		
		printf("[DELTA_GOVENOR] Dying inside ...\n");
		return;
	}
	
	printf("[DELTA_GOVENOR] Delta Govenor service has successfully opened Sigma log file\n");
	printf("[DELTA_GOVENOR] Current timestamp is %lu\n", (unsigned long) time(NULL));
	
	printf("[DELTA_GOVENOR] Checking integrity of Alpha layer (" ALPHA_LAYER_DEVICE ") ...\n");
	system("sudo fsck " ALPHA_LAYER_DEVICE);
	
	printf("[DELTA_GOVENOR] Mounting Alpha layer ...\n");
	system("sudo mount " ALPHA_LAYER_DEVICE " /alpha");
}

#define EXPECTED_FRAMERATE 60

static minigl_surface_t* logo_surface = (minigl_surface_t*) 0;

void quit(void) {
	if (logo_surface) minigl_remove_surface(logo_surface);
}

void main(void) {
	printf("[DELTA_GOVENOR] Delta Govenor started, showing loading screen ...\n");
	minigl_setup(quit);
	
	uint32_t width  = minigl_width ();
	uint32_t height = minigl_height();
	
	logo_surface = minigl_new_image_surface("/kappa/services/delta-govenor/logo.ipx"); // width and height of image must be equal
	float logo_surface_size = 1.0;
	
	float logo_surface_width = logo_surface_size * (float) height / width;
	float logo_surface_height = logo_surface_size;
	
	if (height > width) {
		logo_surface_width = logo_surface_size;
		logo_surface_height = logo_surface_size * (float) width / height;
	}
	
	float seconds = 0.0;
	while (1) {
		seconds += 1.0 / EXPECTED_FRAMERATE;
		minigl_clear(0.0, 0.0, 0.0, 1.0);
		
		float animation = sqrt((seconds / 2) * (seconds / 2)); // take half a second to complete animation
		minigl_draw_surface(logo_surface, 0.0, animation / 2 - 0.5, 0.0, logo_surface_width, logo_surface_height, 1.0, 1.0, 1.0, animation);
		
		minigl_flip();
		if (animation >= 0.99) break;
	}
	
	boot_process();
	printf("[DELTA_GOVENOR] Boot process finished with success, fading out loading screen ...\n";
	
	while (1) {
		seconds -= 1.0 / EXPECTED_FRAMERATE;
		minigl_clear(0.0, 0.0, 0.0, 1.0);
		
		float animation = sqrt((seconds / 2) * (seconds / 2)); // take half a second to complete animation
		minigl_draw_surface(logo_surface, 0.0, animation / 2 - 0.5, 0.0, logo_surface_width, logo_surface_height, 1.0, 1.0, 1.0, animation);
		
		minigl_flip();
		if (animation <= 0.01) break;
	}
	
	printf("[DELTA_GOVENOR] No more need for MiniGL, quitting ...\n");
	minigl_quit();
	
	printf("[DELTA_GOVENOR] Looking for WPA Omega config file (" WPA_OMEGA_CONFIG_PATH ") ...\n");
	if (access(WPA_OMEGA_CONFIG_PATH, F_OK) == -1) {
		printf("[DELTA_GOVENOR] WPA Omega config file was not found, booting into Kappa Setup ...");
		system("sudo /kappa/setup/aqua");
	}
	
	printf("[DELTA_GOVENOR] Booting into consoleOS ...\n");
	system("sudo /alpha/consoleos/aqua");
}
