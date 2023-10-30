#include "ext.h"

#include <string.h>

#include "defines.h"
#include "table.h"

#define ext_page(ptr) ((ptr) / PAGE_SIZE)
#define ext_part(ptr) ((ptr) % PAGE_SIZE)
#define ext_space(ptr) (PAGE_SIZE - ext_part(ptr))

ext_t ext_insert(db_table *table, const void *data, const uint64_t len) {
	// Find new location
	uint64_t end_ptr = table->cmeta.ext_end_ptr;
	void *ext_page = (ext_space(end_ptr) < len || ext_part(end_ptr) == 0) ?
		table_new_ext_page(table, NULL) :
		table_get_ext_page(table, ext_page(end_ptr));
	void *dest = ext_page + ext_part(end_ptr);

	// Write to page
	memcpy(dest, data, len);
	ext_t loc = {
		.ptr = end_ptr,
		.len = len
	};

	// Increment end
	table->cmeta.ext_end_ptr += len;

	return loc;
}

void ext_access(db_table *table, ext_t *locator, void *buf) {
	void *page = table_get_ext_page(table, ext_page(locator->ptr));
	void *src = page + ext_part(locator->ptr);
	memcpy(buf, src, locator->len);
}
