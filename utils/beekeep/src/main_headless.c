/*
  main_headless.c
  beekeep - headless (no GTK) version

  Command-line only scene loader for testing without GTK GUI crashes
*/

#include <stdio.h>
#include <string.h>

#include "files.h"
#include "net_protected.h"

#include "app.h"
#include "dot.h"
#include "json.h"

int main (int argc, char **argv)
{
  char path[64];
  char filename[64];
  char ext[16];
  bool arg = 0;

  setbuf(stdout, NULL);

  if(argc < 2) {
    printf("\r\n usage: beekeep-headless <scene.scn>\r\n");
    printf("\r\n loads a scene file and outputs JSON representation\r\n");
    return 1;
  } else {
    arg = 1;
  }
  
  if(arg) {
    strcpy(path, argv[1]);
    strcpy(filename, argv[1]);
    // scan_ext is not available, do it manually
    char* dot = strrchr(filename, '.');
    if(dot) {
      strcpy(ext, dot);
    } else {
      strcpy(ext, "");
    }
  }

  app_init();
  app_launch(1);

  // Initialize working directory
  if(arg) {
    // set working directory BEFORE loading scene
    strcpy(workingDir, path);
    // strip filename manually
    int i;
    for(i = (int)strlen(workingDir) - 1; i >= 0; i--) {
      if(workingDir[i] == '/') {
        workingDir[i+1] = 0;
        break;
      }
    }
    
    // Extract just the filename from the path
    char* slash = strrchr(filename, '/');
    if(slash) {
      strcpy(filename, slash + 1);
    }
    
    printf("\r\n Loading scene: %s", filename);
    printf("\r\n Working directory: %s", workingDir);
    
    if(strcmp(ext, ".scn") == 0) {
      u8 ret = files_load_scene_name(filename);
      if(ret == 0) {
        printf("\r\n Scene loaded successfully\r\n");
        // Output JSON
        net_write_json_native("output.json");
        printf("\r\n Exported to output.json\r\n");
      } else {
        printf("\r\n ERROR: Failed to load scene\r\n");
        return 1;
      }
    }
    else if(strcmp(ext, ".json") == 0) {
      net_read_json_native(filename);
      printf("\r\n JSON loaded\r\n");
    }
    else {
      printf("\r\n ERROR: Unknown file type (expected .scn or .json)\r\n");
      return 1;
    }
  }
  
  printf("\r\n Done.\r\n");
  return 0;
}
