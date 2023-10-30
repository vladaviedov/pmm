#pragma once

#include "../db/table.h"
#include "../db/defines.h"

// Package status
typedef enum {
	PKG_MISSING,
	PKG_OLD,
	PKG_OK
} pkg_status;

// Package layout
typedef struct {
	ext_t name;
	ext_t group;
	pkg_status status;
} pkg;

// Alias
typedef db_table pkg_table;

/**
 * @brief Open table containing packages.
 *
 * @param[in] file - File name.
 * @return Table object.
 */
pkg_table *pkg_open(const char *file);

/**
 * @brief Save package database table.
 *
 * @param[in] table - Table object.
 * @return Status code.
 */
int pkg_save(pkg_table *table);

/**
 * @brief Add a new package to the database.
 *
 * @param[in] table - Table object.
 * @param[in] name - Name of package.
 * @return Status code.
 */
int pkg_add(pkg_table *table, const char *name);

int pkg_remove(char *name);

int pkg_print_info(char *name);

/**
 * @brief Check that packages exist and update state.
 *
 * @param[in] table - Table object.
 * @return Status code.
 */
int pkg_check(pkg_table *table);

/**
 * @brief Print all packages stored in database.
 *
 * @param[in] table - Table object.
 * @return Status code.
 */
int pkg_print_all(pkg_table *table);

/**
 * @brief Sync installed packages to wanted packages.
 *
 * @return Status code.
 */
int pkg_sync(pkg_table *table);
