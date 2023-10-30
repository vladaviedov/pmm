#include "pkg.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <openssl/md5.h>

#include "defines.h"

#include "../util/color.h"
#include "../util/vector.h"
#include "../client/client.h"
#include "../db/defines.h"
#include "../db/table.h"
#include "../db/btree.h"
#include "../db/ext.h"

pkg_status check_status(const client *cl, const char *pkg);

pkg_table *pkg_open(const char *file) {
	pkg_table *table = table_open(file, PKG);
	if (table == NULL) {
		return NULL;
	}

	if (table->cmeta.root_page == INVALID_VAL) {
		btree_init(table, sizeof(pkg));
	}

	return table;
}

int pkg_save(pkg_table *table) {
	if (table == NULL) {
		return -1;
	}

	return table_save(table);
}

int pkg_add(pkg_table *table, const char *name) {
	if (table == NULL) {
		return -1;
	}
	
	// Check if package exists
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
	pkg record = {
		.name = ext_insert(table, name, strlen(name)),
		.group = { .ptr = INVALID_EXT, .len = 0 },
		.status = check_status(cl, name)
	};

	return btree_insert(table, &hash, &record);
}

int pkg_check(pkg_table *table) {
	if (table == NULL) {
		return -1;
	}

	const client *cl = client_get();
	btree_cursor *iter = btree_iter(table);
	while (!iter->end) {
		pkg *package = btree_next(iter);

		// Get name
		char name[package->name.len + 1];
		ext_access(table, &package->name, name);
		name[package->name.len] = '\0';

		// Check if package exists
		if (!cl->exists(name)) {
			fprintf(stderr, "warning: package %s does not exist\n", name);
			continue;
		}

		// Get status
		package->status = check_status(cl, name);
	}

	return 0;
}

int pkg_print_all(pkg_table *table) {
	if (table == NULL) {
		return -1;
	}

	uint32_t missing = 0;
	uint32_t old = 0;
	uint32_t ok = 0;

	btree_cursor *iter = btree_iter(table);
	while (!iter->end) {
		pkg *package = btree_next(iter);

		// Set color
		switch (package->status) {
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
		char name[package->name.len + 1];
		ext_access(table, &package->name, name);
		name[package->name.len] = '\0';
		printf("%s", name);

		// Print group
		if (package->group.ptr != INVALID_EXT) {
			char group[package->group.len + 1];
			ext_access(table, &package->group, group);
			group[package->group.len] = '\0';
			printf(" (%s)", group);
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

int pkg_sync(pkg_table *table) {
	if (table == NULL) {
		return -1;
	}

	const client *cl = client_get();
	btree_cursor *iter = btree_iter(table);

	vector *installs = vec_new(sizeof(char *));
	vector *updates = vec_new(sizeof(pkg *));

	while (!iter->end) {
		pkg *package = btree_next(iter);

		// Get name
		char *name = malloc(package->name.len + 1);
		ext_access(table, &package->name, name);
		name[package->name.len] = '\0';

		// Update status
		package->status = check_status(cl, name);

		if (package->status == PKG_MISSING) {
			vec_push(installs, &name);
			vec_push(updates, &package);
		}
	}

	// Do install
	int res = cl->install(installs->raw_array, installs->count);
	if (res == 0) {
		for (uint32_t i = 0; i < updates->count; i++) {
			pkg *package = *(pkg **)vec_at(updates, i);
			package->status = PKG_OK;
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

pkg_status check_status(const client *cl, const char *pkg) {
	return (!cl->installed(pkg)) ? PKG_MISSING
		: (cl->outdated(pkg)) ? PKG_OLD
		: PKG_OK;
}
