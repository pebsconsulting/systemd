/* SPDX-License-Identifier: LGPL-2.1+ */
/***
  This file is part of systemd.

  Copyright 2017 Zbigniew Jędrzejewski-Szmek
***/

#include "sd-path.h"

#include "conf-files.h"
#include "def.h"
#include "escape.h"
#include "fileio.h"
#include "log.h"
#include "path-lookup.h"

static int environment_dirs(char ***ret) {
        _cleanup_strv_free_ char **dirs = NULL;
        _cleanup_free_ char *c = NULL;
        int r;

        dirs = strv_split_nulstr(CONF_PATHS_NULSTR("environment.d"));
        if (!dirs)
                return -ENOMEM;

        /* ~/.config/systemd/environment.d */
        r = sd_path_home(SD_PATH_USER_CONFIGURATION, "environment.d", &c);
        if (r < 0)
                return r;

        r = strv_extend_front(&dirs, c);
        if (r < 0)
                return r;

        *ret = TAKE_PTR(dirs);
        return 0;
}

static int load_and_print(void) {
        _cleanup_strv_free_ char **dirs = NULL, **files = NULL, **env = NULL;
        char **i;
        int r;

        r = environment_dirs(&dirs);
        if (r < 0)
                return r;

        r = conf_files_list_strv(&files, ".conf", NULL, 0, (const char **) dirs);
        if (r < 0)
                return r;

        /* This will mutate the existing environment, based on the presumption
         * that in case of failure, a partial update is better than none. */

        STRV_FOREACH(i, files) {
                r = merge_env_file(&env, NULL, *i);
                if (r == -ENOMEM)
                        return r;
        }

        STRV_FOREACH(i, env) {
                char *t;
                _cleanup_free_ char *q = NULL;

                t = strchr(*i, '=');
                assert(t);

                q = shell_maybe_quote(t + 1, ESCAPE_BACKSLASH);
                if (!q)
                        return log_oom();

                printf("%.*s=%s\n", (int) (t - *i), *i, q);
        }

        return 0;
}

int main(int argc, char *argv[]) {
        int r;

        log_parse_environment();
        log_open();

        if (argc > 1) {
                log_error("This program takes no arguments.");
                return EXIT_FAILURE;
        }

        r = load_and_print();
        if (r < 0)
                log_error_errno(r, "Failed to load environment.d: %m");

        return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
