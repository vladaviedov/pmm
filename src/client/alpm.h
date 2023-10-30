#pragma once

/**
 * @brief Check if package exists within pacman's repositories.
 *
 * @param[in] name - Package name.
 * @return Boolean result.
 */
int pmm_alpm_exists(const char *name);

/**
 * @brief Check if package is currently installed.
 *
 * @param[in] name - Package name.
 * @return Boolean result.
 */
int pmm_alpm_installed(const char *name);

/**
 * @brief Check if the local package is out of date.
 *
 * @param[in] name - Package name.
 * @return Boolean result.
 */
int pmm_alpm_outdated(const char *name);
