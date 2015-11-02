/*
 * libopenemv - a library to work with EMV family of smart cards
 * Copyright (C) 2015 Dmitry Eremin-Solenikov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "openemv/config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
//#include <libconfig.h>
typedef struct _config_t{
    
}config_t;

static inline char* config_error_text(const config_t *config)
{
    return "";
}
static inline void config_destroy(const config_t *config)
{
//    return "";
}
static inline void config_init(const config_t *config)
{
    //    return "";
}
static inline int config_lookup_string(const config_t *config, const char*x , char **xx)
{
    return 0;
}
static inline int config_read_file(const config_t *config, const char*x)
{
    return 0;
}
#define OPENEMV_CONFIG_DIR
#define CONFIG_TRUE 1

#ifndef LIBCONFIG_VER_MAJOR
static void openemv_config_error(const config_t *config)
{
	fprintf(stderr, "libconfig: %s\n",
			config_error_text(config));
}
#else
static void openemv_config_error(const config_t *config)
{
	fprintf(stderr, "%s:%d: %s\n",
			config_error_file(config),
			config_error_line(config),
			config_error_text(config));
}
#endif

static config_t *_openemv_config;

static void openemv_init_config(void)
{
	config_t *config;
	int ret;
	const char *fname;

	config = malloc(sizeof(*_openemv_config));
	config_init(config);
	fname = getenv("OPENEMV_CONFIG");
	if (!fname)
		fname = OPENEMV_CONFIG_DIR "config.txt";
	ret = config_read_file(config, fname);
	if (ret != CONFIG_TRUE) {
		openemv_config_error(config);
		config_destroy(config);
		free(config);
	} else
		_openemv_config = config;
}

const char *openemv_config_get(const char *path)
{
	const char *value;
	int ret;

	if (!_openemv_config)
		openemv_init_config();

	if (!_openemv_config)
		return NULL;

	ret = config_lookup_string(_openemv_config, path, &value);

	if (ret != CONFIG_TRUE)
		return NULL;

	return value;
}
