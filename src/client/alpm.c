#include "alpm.h"

#include <string.h>

#include <alpm.h>
#include <alpm_list.h>

// TODO: implement properly
#define make_handle() alpm_initialize("/", "/var/lib/pacman", NULL)
#define delete_handle(handle) alpm_release(handle)

alpm_pkg_t *find_local(alpm_handle_t *handle, char *name);
alpm_pkg_t *find_repos(alpm_handle_t *handle, char *name);
void setup_alpm(alpm_handle_t *handle);

int pmm_alpm_exists(char *name) {
	alpm_handle_t *handle = make_handle();
	setup_alpm(handle);

	/* int found = 0; */
	/* for (alpm_list_t *i = alpm_get_syncdbs(handle); i; i = alpm_list_next(i)) { */
	/* 	alpm_pkg_t *pkg = alpm_db_get_pkg(i->data, name); */
	/* 	if (pkg != NULL) { */
	/* 		found = 1; */
	/* 		break; */
	/* 	} */

	/* 	for (alpm_list_t *j = alpm_db_get_pkgcache(i->data); j; j = alpm_list_next(j)) { */
	/* 		if (strcmp(alpm_pkg_get_name(j->data), name) == 0) { */
	/* 			found = 1; */
	/* 			break; */
	/* 		} */
	/* 	} */
	/* } */

	alpm_pkg_t *pkg = find_repos(handle, name);
	int result = (pkg != NULL);

	delete_handle(handle);
	return result;
}

int pmm_alpm_installed(char *name) {
	alpm_handle_t *handle = make_handle();
	setup_alpm(handle);

	/* int found = 0; */
	/* for (alpm_list_t *i = alpm_db_get_pkgcache(alpm_get_localdb(handle)); i; i = alpm_list_next(i)) { */
	/* 	if (strcmp(alpm_pkg_get_name(i->data), name) == 0) { */
	/* 		found = 1; */
	/* 		break; */
	/* 	} */
	/* } */

	alpm_pkg_t *pkg = find_local(handle, name);
	int result = (pkg != NULL);

	delete_handle(handle);
	return result;
}

int pmm_alpm_outdated(char *name) {
	alpm_handle_t *handle = make_handle();
	setup_alpm(handle);

	alpm_pkg_t *pkg = find_local(handle, name);
	if (pkg == NULL) {
		return 0;
	}

	int result = (alpm_sync_get_new_version(pkg, alpm_get_syncdbs(handle)) != NULL);

	delete_handle(handle);
	return result;
}

/** Private functions */

/**
 * @brief Find package in local repository.
 *
 * @param[in] handle - ALPM handle from make_handle.
 * @param[in] name - Package name.
 * @return ALPM package or NULL if not found.
 */
alpm_pkg_t *find_local(alpm_handle_t *handle, char *name) {
	alpm_db_t *local_db = alpm_get_localdb(handle);
	return alpm_db_get_pkg(local_db, name);
}

/**
 * @brief Find package in sync repositories.
 *
 * @param[in] handle - ALPM handle from make_handle.
 * @param[in] name - Package name.
 * @return ALPM package or NULL if not found.
 */
alpm_pkg_t *find_repos(alpm_handle_t *handle, char *name) {
	alpm_list_t *syncs = alpm_get_syncdbs(handle);
	for (alpm_list_t *i = syncs; i != NULL; i = alpm_list_next(i)) {
		alpm_pkg_t *pkg_search = alpm_db_get_pkg(i->data, name);
		if (pkg_search != NULL) {
			return pkg_search;
		}
	}

	return NULL;
}

void setup_alpm(alpm_handle_t *handle) {
	alpm_register_syncdb(handle, "core", 0);
	alpm_register_syncdb(handle, "extra", 0);
	alpm_register_syncdb(handle, "multilib", 0);
}
