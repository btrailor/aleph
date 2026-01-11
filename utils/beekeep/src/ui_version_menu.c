#include "ui_version_menu.h"
#include <stdio.h>

// Available BEES versions for export
static const version_info_t available_versions[] = {
    {0, 7, 0, "BEES 0.7.0"},
    {0, 7, 1, "BEES 0.7.1"},
    {0, 7, 2, "BEES 0.7.2 (current)"},
    {0, 8, 0, "BEES 0.8.0 (experimental)"},
    {1, 0, 0, "BEES 1.0.0 (future)"}
};

static const int num_versions = sizeof(available_versions) / sizeof(available_versions[0]);
static int selected_version_index = 2; // Default to current version (0.7.2)

static void version_menu_select(GtkWidget* widget, gpointer data) {
    int* pVersionIndex = (int*)data;
    selected_version_index = *pVersionIndex;
    printf("\r\n version selection: %s", available_versions[*pVersionIndex].display_name);
}

GtkWidget* create_version_menu(void) {
    GtkWidget* menu;
    GtkWidget* child;
    int i;
    
    menu = gtk_menu_new();
    
    for(i = 0; i < num_versions; i++) {
        child = gtk_menu_item_new_with_label(available_versions[i].display_name);
        gtk_menu_attach(GTK_MENU(menu), child, 0, 1, i+1, i+2);
        // Show the widget (required for GTK menus)
        gtk_widget_show(child);
        
        // Store the index directly since we're using a static array
        static int version_indices[5]; // Matches num_versions
        version_indices[i] = i;
        
        g_signal_connect(child, 
                        "activate", 
                        G_CALLBACK(version_menu_select), 
                        (gpointer)&(version_indices[i]));
    }
    
    return menu;
}

const version_info_t* get_selected_version(void) {
    if (selected_version_index >= 0 && selected_version_index < num_versions) {
        return &available_versions[selected_version_index];
    }
    return &available_versions[2]; // Default to current version
}