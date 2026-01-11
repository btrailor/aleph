#ifndef _UI_VERSION_MENU_H_
#define _UI_VERSION_MENU_H_

#include <gtk/gtk.h>

// Version information structure
typedef struct {
    int maj;
    int min;
    int rev;
    const char* display_name;
} version_info_t;

// Create version export menu
GtkWidget* create_version_menu(void);

// Get selected version info
const version_info_t* get_selected_version(void);

#endif