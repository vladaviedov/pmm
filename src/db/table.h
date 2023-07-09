#pragma once

#include <stdint.h>

#include "defines.h"

// Page pointer
typedef struct {
	page_t pg_num;
	void *raw_data;
} db_page;

// Cache manager
typedef struct {
	uint32_t page_count;
	uint32_t alloc_count;
	db_page *data;
} db_cache;

// Metadata information
typedef struct {
	uint32_t table_version;
	uint32_t total_pages;
	uint32_t ext_start;
	uint64_t ext_end_ptr;
	page_t root_page;
} db_meta;

// Database table
typedef struct {
	// Database file info
	int fd;
	uint32_t fsize;
	db_meta fmeta;
	// Cache
	db_meta cmeta;
	db_cache *norm_cache;
	db_cache *ext_cache;
} db_table;

/**
 * @brief Load database table.
 *
 * @param[in] file - Filename.
 * @param[in] table_ver - Current expected table version.
 * @return New db_table object.
 */
db_table *table_open(const char *file, uint32_t table_ver);

/**
 * @brief Save database to disk & close database object.
 *
 * @param[in] table - Table object.
 * @return Success code.
 */
int table_save(db_table *table);

/**
 * @brief Load a normal page from database.
 *
 * @param[in] table - Table object.
 * @param[in] page_num - Page number to retrieve.
 * @return Database page (size = PAGE_SIZE).
 */
void *table_get_norm_page(db_table *table, page_t page_num);

/**
 * @brief Load a extension page from database.
 *
 * @param[in] table - Table object.
 * @param[in] page_num - Page number to retrieve.
 * @return Database page (size = PAGE_SIZE).
 */
void *table_get_ext_page(db_table *table, page_t page_num);

/**
 * @brief Create a new normal database page.
 *
 * @param[in] table - Table object.
 * @param[out] index - New page index (if not NULL).
 * @return Database page (size = PAGE_SIZE).
 */
void *table_new_norm_page(db_table *table, page_t *index);

/**
 * @brief Create a new extension database page.
 *
 * @param[in] table - Table object.
 * @param[out] index - New page index (if not NULL).
 * @return Database page (size = PAGE_SIZE).
 */
void *table_new_ext_page(db_table *table, page_t *index);
