#pragma once

#include "defines.h"

typedef enum {
	PKG_MISSING,
	PKG_OLD,
	PKG_OK
} db_pkg_status;

typedef struct {
	ext_t name;
	ext_t group;
	db_pkg_status status;
} db_pkg;

/**
 * @brief Open pkg database table.
 *
 * @return Status code.
 */
int pkg_open(void);

/**
 * @brief Save pkg database table.
 *
 * @return Status code.
 */
int pkg_save(void);

/**
 * @brief Add a new package to the database.
 *
 * @param[in] name - Name of package.
 * @return Status code.
 */
int pkg_add(char *name);

int pkg_remove(char *name);

int pkg_print_info(char *name);

/**
 * @brief Print all packages stored in database.
 *
 * @return Status code.
 */
int pkg_print_all(void);

int pkg_sync(void);
