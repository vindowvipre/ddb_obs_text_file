#include <stdio.h>
#include <string.h>
#include <deadbeef/deadbeef.h>

static DB_functions_t *deadbeef;
static DB_misc_t plugin;

static int handle_event(uint32_t event, uintptr_t ctx, uint32_t p1, uint32_t p2) {

    if (deadbeef->conf_get_int("obs_text_file.enabled", 1) != 1) {
        return 0;
    }

    if (event == DB_EV_SONGCHANGED | event == DB_EV_PAUSED | event == DB_EV_STOP) {

        char path_to_file[512];

        deadbeef->conf_get_str("obs_text_file.path", "obs_text_file.txt", path_to_file, sizeof(path_to_file));
        
        char format_str[512];

        deadbeef->conf_get_str("obs_text_file.formatting", "%artist% - %title%", format_str, sizeof(format_str));

        // p1 is 1 when pausing
        if (deadbeef->conf_get_int("obs_text_file.emptyifpause", 0) == 1 && (event == DB_EV_STOP | p1 == 1)) {
            strncpy(format_str, "", sizeof(format_str));
        }
        
        // title formatting 
        char* fmt = deadbeef->tf_compile(format_str);
        // 512 characters is an arbitrary number 
        char out[512];
        // deadbeef api black magic 
        DB_playItem_t *it = deadbeef->streamer_get_playing_track_safe();
        ddb_tf_context_t ctx = {
            ._size = sizeof(ddb_tf_context_t),
            .flags = 0,
            .it = it,
            .plt = NULL,
            .idx = 0,
            .id = 0,
            .iter = PL_MAIN,
        };
        deadbeef->tf_eval(&ctx, fmt, out, sizeof(out));

        FILE *file = fopen(path_to_file, "w");
        fprintf(file, "%s\n", out);
        fclose(file);
    }

    return 0;
}

static const char settings_dlg[] =
    "property \"Enable\" checkbox obs_text_file.enabled 1;\n"
    "property \"Empty string when not playing\" checkbox obs_text_file.emptyifpause 0;\n"
    "property \"Text format\" entry obs_text_file.formatting \"%artist% - %title%\";\n"
    "property \"Path to file\" entry obs_text_file.path \"obs_text_file.txt\";\n"    
;

static DB_misc_t plugin = {
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 0,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.id = "obs_text_file",
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.name = "OBS Text File",
    .plugin.descr = "Writes the current playing song to a text file (for OBS streams)",
    .plugin.copyright =         
        "OBS text file plugin for DeaDBeeF Player\n"
        "Copyright (C) 2022\n"
        "\n"
        "This software is provided 'as-is', without any express or implied\n"
        "warranty.  In no event will the authors be held liable for any damages\n"
        "arising from the use of this software.\n"
        "\n"
        "Permission is granted to anyone to use this software for any purpose,\n"
        "including commercial applications, and to alter it and redistribute it\n"
        "freely, subject to the following restrictions:\n"
        "\n"
        "1. The origin of this software must not be misrepresented; you must not\n"
        " claim that you wrote the original software. If you use this software\n"
        " in a product, an acknowledgment in the product documentation would be\n"
        " appreciated but is not required.\n"
        "\n"
        "2. Altered source versions must be plainly marked as such, and must not be\n"
        " misrepresented as being the original software.\n"
        "\n"
        "3. This notice may not be removed or altered from any source distribution.\n",
    .plugin.website = "http://www.github.com/vindowvipre/ddb_obs_text_file",
    .plugin.message = handle_event,
    .plugin.configdialog = settings_dlg,
};

DB_plugin_t *ddb_obs_text_file_load(DB_functions_t *api) {
    deadbeef = api;
    return &plugin.plugin;
}