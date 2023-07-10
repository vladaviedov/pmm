#include "pkg.h"

#include <bits/stdint-uintn.h>
#include <stdio.h>
#include <openssl/md5.h>

#include "defines.h"
#include "table.h"
#include "btree.h"
#include "ext.h"

#include "../common.h"
#include "../client/client.h"

static db_table *table = NULL;

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
		fprintf(stderr, "table is not open\n");
		return -1;
	}

	return table_save(table);
}

int pkg_add(char *name) {
	if (table == NULL) {
		fprintf(stderr, "table is not open\n");
		return -1;
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
		.status = (!cl->installed(name))
			? PKG_MISSING
			: (cl->outdated(name) ? PKG_OLD : PKG_OK)
	};

	return btree_insert(table, &hash, &pkg);
}

int pkg_print_all(void) {
	if (table == NULL) {
		fprintf(stderr, "table is not open\n");
		return -1;
	}

	btree_cursor *iter = btree_iter(table);
	uint32_t missing = 0;
	uint32_t old = 0;
	uint32_t ok = 0;
	while (iter->end == 0) {
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

	return 0;
}
