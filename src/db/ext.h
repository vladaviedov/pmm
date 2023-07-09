#pragma once

#include "defines.h"
#include "table.h"

/**
 * @brief Insert data into the extension section.
 *
 * @param[in] table - Table object.
 * @param[in] data - Data to insert.
 * @return Ext page resource locator.
 */
ext_t ext_insert(db_table *table, void *data, uint64_t len);

/**
 * @brief Access data pointed to by a locator.
 *
 * @param[in] table - Table object.
 * @param[in] locator - Ext page locator.
 * @param[out] buf - Buffer to copy data to.
 */
void ext_access(db_table *table, ext_t *locator, void *buf);
