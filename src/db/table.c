#include "table.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "defines.h"
#include "../util/vector.h"

// Get normal page count
#define norm_count(meta) meta.ext_start
// Get extension page count
#define ext_count(meta) (meta.total_pages - meta.ext_start)
// Get location of page in file
#define locate_page(page) (sizeof(db_meta) + (page) * PAGE_SIZE)

int flush_cache(vector *cache, int fd, uint32_t offset);
db_page *load_to_cache(vector *cache, int fd, page_t page_num);

db_table *table_open(const char *file, uint32_t table_ver) {
	db_table *t = malloc(sizeof(db_table));

	// Open database file
	int fd = open(file, O_RDWR | O_CREAT, 0644);
	if (fd < 0) {
		fprintf(stderr, "failed to open database table\n");
		return NULL;
	}
	t->fd = fd;
	t->fsize = lseek(fd, 0, SEEK_END);

	if (t->fsize == 0) {
		// Fresh file: create new metadata
		t->fmeta.table_version = table_ver;
		t->fmeta.total_pages = 0;
		t->fmeta.ext_start = 0;
		t->fmeta.root_page = INVALID_VAL;
		t->fmeta.ext_end_ptr = 0;
	} else {
		// Load saved table version
		uint32_t file_table_ver;
		lseek(fd, 0, SEEK_SET);
		int64_t bytes = read(fd, &file_table_ver, sizeof(uint32_t));
		if (bytes != sizeof(uint32_t)) {
			fprintf(stderr, "failed to read table version from file\n");
			return NULL;
		}
		
		// Version check
		if (table_ver < file_table_ver) {
			fprintf(stderr, "stored database has old table: needs upgrade\n");
			return NULL;
		} else if (table_ver > file_table_ver) {
			fprintf(stderr, "unrecognized table version number\n");
			return NULL;
		}

		// Okay, can load metadata now
		lseek(fd, 0, SEEK_SET);
		bytes = read(fd, &t->fmeta, sizeof(db_meta));
		if (bytes != sizeof(db_meta)) {
			fprintf(stderr, "failed to read table metadata\n");
			return NULL;
		}
	}
	t->cmeta = t->fmeta;

	// Allocate cache objects
	t->norm_cache = vec_new(sizeof(db_page));
	t->ext_cache = vec_new(sizeof(db_page));

	return t;
}

int table_save(db_table *table) {
	if (table == NULL) {
		return -1;
	}

	// Write metadata
	lseek(table->fd, 0, SEEK_SET);
	if (write(table->fd, &table->cmeta, sizeof(db_meta)) != sizeof(db_meta)) {
		fprintf(stderr, "failed to write metadata\n");
		return -1;
	}

	// Move ext pages if spaces needed
	uint32_t norm_delta = norm_count(table->cmeta) - norm_count(table->fmeta);
	if (norm_delta > 0 && table->fmeta.total_pages > 0) {
		uint8_t buf[PAGE_SIZE];
		for (uint32_t i = table->fmeta.total_pages - 1; i >= table->fmeta.ext_start; i--) {
			lseek(table->fd, locate_page(i), SEEK_SET);
			if (read(table->fd, buf, PAGE_SIZE) != PAGE_SIZE) {
				fprintf(stderr, "failed to read page\n");
				return -1;
			}

			lseek(table->fd, locate_page(i + norm_delta), SEEK_SET);
			if (write(table->fd, buf, PAGE_SIZE) != PAGE_SIZE) {
				fprintf(stderr, "failed to write page\n");
				return -1;
			}
		}
	}

	// Flush caches to file
	flush_cache(table->norm_cache, table->fd, 0);
	flush_cache(table->ext_cache, table->fd, table->cmeta.ext_start);

	// Cleanup
	vec_free(table->norm_cache);
	vec_free(table->ext_cache);
	close(table->fd);
	free(table);

	return 0;
}

void *table_get_norm_page(db_table *table, page_t page_num) {
	if (page_num >= table->cmeta.ext_start) {
		fprintf(stderr, "tried to access page outside of database\n");
		exit(EXIT_FAILURE);
	}

	// Check cache
	for (uint32_t i = 0; i < table->norm_cache->count; i++) {
		db_page *page = vec_at(table->norm_cache, i);
		if (page_num == page->pg_num) {
			return page->raw_data;
		}
	}

	// Cache miss
	// fmeta does not need to be checked
	db_page *loaded_page = load_to_cache(table->norm_cache, table->fd, page_num);
	if (loaded_page == NULL) {
		fprintf(stderr, "failed to load page from cache\n");
		exit(EXIT_FAILURE);
	}

	return loaded_page->raw_data;
}

void *table_get_ext_page(db_table *table, page_t page_num) {
	if (page_num >= table->cmeta.total_pages) {
		fprintf(stderr, "tried to access page outside of database\n");
		exit(EXIT_FAILURE);
	}

	// Check cache
	for (uint32_t i = 0; i < table->ext_cache->count; i++) {
		db_page *page = vec_at(table->ext_cache, i);
		if (page_num == page->pg_num) {
			return page->raw_data;
		}
	}

	// Cache miss
	uint32_t page_in_file = table->fmeta.ext_start + page_num;
	db_page *loaded_page = load_to_cache(table->ext_cache, table->fd, page_in_file);
	loaded_page->pg_num = page_num;

	if (loaded_page == NULL) {
		fprintf(stderr, "failed to load page from cache\n");
		exit(EXIT_FAILURE);
	}

	return loaded_page->raw_data;
}

void *table_new_norm_page(db_table *table, page_t *index) {
	// Create page in cache
	db_page new_page = {
		.pg_num = table->cmeta.ext_start,
		.raw_data = calloc(1, PAGE_SIZE)
	};
	vec_push(table->norm_cache, &new_page);

	// Update cmeta
	table->cmeta.ext_start++;
	table->cmeta.total_pages++;
	
	if (index != NULL) {
		*index = new_page.pg_num;
	}
	return new_page.raw_data;
}

void *table_new_ext_page(db_table *table, page_t *index) {
	// Create page in cache
	db_page new_page = {
		.pg_num = table->cmeta.total_pages - table->cmeta.ext_start,
		.raw_data = calloc(1, PAGE_SIZE)
	};
	db_page *inserted = vec_push(table->ext_cache, &new_page);
	if (inserted == NULL) {
		fprintf(stderr, "failed to create new page in cache\n");
		exit(EXIT_FAILURE);
	}

	// Update cmeta
	table->cmeta.total_pages++;
	
	if (index != NULL) {
		*index = inserted->pg_num;
	}
	return inserted->raw_data;
}

/** Private functions */

/**
 * @brief Save table cache to file.
 *
 * @param[in] cache - Table cache.
 * @param[in] fd - File descriptior.
 * @param[in] offset - Write page offset.
 * @return Success code.
 */
int flush_cache(vector *cache, int fd, uint32_t offset) {
	for (uint32_t i = 0; i < cache->count; i++) {
		db_page *page = vec_at(cache, i);
		uint64_t place = locate_page(page->pg_num + offset);
		lseek(fd, place, SEEK_SET);
		/* lseek(fd, locate_page(page->pg_num + offset), SEEK_SET); */
		if (write(fd, page->raw_data, PAGE_SIZE) != PAGE_SIZE) {
			fprintf(stderr, "failed to flush page\n");
			return -1;
		}

		// Free cache object
		free(page->raw_data);
	}

	return 0;
}

/**
 * @brief Load page from file to cache.
 *
 * @param[in] cache - Table cache.
 * @param[in] fd - Table file descriptior.
 * @param[in] page_num - Page number.
 * @return Pointer to db_page object in cache with requested page.
 */
db_page *load_to_cache(vector *cache, int fd, page_t page_num) {
	void *page_buffer = malloc(PAGE_SIZE);

	// Read page
	lseek(fd, locate_page(page_num), SEEK_SET);
	if (read(fd, page_buffer, PAGE_SIZE) != PAGE_SIZE) {
		fprintf(stderr, "failed to read file\n");
		return NULL;
	}

	db_page page = {
		.pg_num = page_num,
		.raw_data = page_buffer
	};

	return vec_push(cache, &page);
}
