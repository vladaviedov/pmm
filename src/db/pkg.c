#include "pkg.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <openssl/md5.h>

#include "defines.h"
#include "table.h"
#include "btree.h"
#include "ext.h"

#include "../util/color.h"
#include "../util/vector.h"
#include "../client/client.h"

static db_table *table = NULL;

db_pkg_status update_status(const client *cl, char *pkg);

int pkg_open(void) {
	if (table != NULL) {
		fprintf(stderr, "table is already open\n");
		return -1;
	}

	table = table_open(PKG_TABLE, PKG_TABLE_VER);
	if (table->cmeta.root_page == INVALID_VAL) {
		btree_init(table, sizeof(db_pkg));
	}

	return 0;
}

int pkg_save(void) {
	if (table == NULL) {
		printf("table was not open");
		return -1;
	}

	return table_save(table);
}

int pkg_add(char *name) {
	if (table == NULL) {
		if (pkg_open() < 0) {
			fprintf(stderr, "cannot open table\n");
			return -1;
		}
	}
	
	// Check if client exists
	const client *cl = client_get();
	if (!cl->exists(name)) {
		fprintf(stderr, "package does not exist\n");
		return -1;
	}

	// Generate hash
	md5_t hash;
	// TODO: use a non-deprecated way
	MD5((uint8_t *)name, strlen(name), md5_req_ptr(hash));

	// Create record
	db_pkg pkg = {
		.name = ext_insert(table, name, strlen(name)),
		.group = { .ptr = INVALID_EXT, .len = 0 },
		.status = update_status(cl, name)
	};

	return btree_insert(table, &hash, &pkg);
}

int pkg_print_all(void) {
	if (table == NULL) {
		if (pkg_open() < 0) {
			fprintf(stderr, "cannot open table\n");
			return -1;
		}
	}

	btree_cursor *iter = btree_iter(table);
	uint32_t missing = 0;
	uint32_t old = 0;
	uint32_t ok = 0;
	while (!iter->end) {
		db_pkg *pkg = btree_next(iter);

		// Set color
		switch (pkg->status) {
			case PKG_MISSING:
				printf_color(RED);
				missing++;
				break;
			case PKG_OLD:
				printf_color(YELLOW);
				old++;
				break;
			case PKG_OK:
				printf_color(GREEN);
				ok++;
				break;
		}

		// Print name
		char name_buf[pkg->name.len + 1];
		ext_access(table, &pkg->name, name_buf);
		name_buf[pkg->name.len] = '\0';
		printf("%s", name_buf);

		// Print group
		if (pkg->group.ptr != INVALID_EXT) {
			char group_buf[pkg->group.len + 1];
			ext_access(table, &pkg->group, group_buf);
			group_buf[pkg->group.len] = '\0';
			printf(" (%s)", group_buf);
		}

		putchar('\n');
	}

	uint32_t installed = old + ok;
	uint32_t total = installed + missing;
	printf_color(WHITE);
	printf("Installed: %u / %u | Up to date: %u / %u\n", installed, total, ok, installed);

	free(iter);
	return 0;
}

int pkg_sync(void) {
	if (table == NULL) {
		if (pkg_open() < 0) {
			fprintf(stderr, "cannot open table\n");
			return -1;
		}
	}

	const client *cl = client_get();
	btree_cursor *iter = btree_iter(table);

	vector *installs = vec_new(sizeof(char *));
	vector *updates = vec_new(sizeof(db_pkg *));

	while (!iter->end) {
		db_pkg *pkg = btree_next(iter);

		// Get name
		char *name = malloc(pkg->name.len + 1);
		ext_access(table, &pkg->name, name);
		name[pkg->name.len] = '\0';

		// Update status
		pkg->status = update_status(cl, name);

		if (pkg->status == PKG_MISSING) {
			vec_push(installs, &name);
			vec_push(updates, &pkg);
		}
	}

	// Do install
	int res = cl->install(installs->raw_array, installs->count);
	if (res == 0) {
		for (uint32_t i = 0; i < updates->count; i++) {
			db_pkg *pkg = *(db_pkg **)vec_at(updates, i);
			pkg->status = PKG_OK;
		}
	}

	for (uint32_t i = 0; i < installs->count; i++) {
		free(*(char **)vec_at(installs, i));
	}

	vec_free(installs);
	vec_free(updates);
	return 0;
}

/** Private functions */

db_pkg_status update_status(const client *cl, char *pkg) {
	return (!cl->installed(pkg)) ? PKG_MISSING
		: (cl->outdated(pkg)) ? PKG_OLD
		: PKG_OK;
}
