#pragma once

#include <stdint.h>

typedef struct {
	void *raw_array;
	uint32_t count;

	uint64_t _type_size;
	uint32_t _alloc_count;
} vector;

/**
 * @brief Create new vector.
 *
 * @param[in] type_size - Size of type stored.
 * @return Vector object.
 */
vector *vec_new(uint32_t type_size);

/**
 * @brief Add new element to vector.
 *
 * @param[in] vec - Vector object.
 * @param[in] val - Value to add. Expected to be same size as type_size.
 */
void vec_push(vector *vec, void *val);

/**
 * @brief Get element at position.
 *
 * @param[in] vec - Vector object.
 * @param[in] index - Index to get.
 * @returns Pointer to element.
 */
void *vec_at(vector *vec, uint32_t index);

/**
 * @brief Delete vector.
 *
 * @param[in] vec - Vector object.
 */
void vec_free(vector *vec);
