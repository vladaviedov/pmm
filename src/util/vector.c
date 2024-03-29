#include "vector.h"

#include <stdlib.h>
#include <string.h>

vector *vec_new(uint32_t type_size) {
	vector *vec = malloc(sizeof(vector));

	vec->raw_array = NULL;
	vec->count = 0;

	vec->_type_size = type_size;
	vec->_alloc_count = 0;

	return vec;
}

void *vec_push(vector *vec, void *val) {
	if (vec->raw_array == NULL) {
		vec->_alloc_count = 2;
		vec->raw_array = malloc(vec->_type_size * vec->_alloc_count);
	}

	if (vec->count == vec->_alloc_count) {
		vec->_alloc_count *= 2;
		vec->raw_array = realloc(vec->raw_array, vec->_type_size * vec->_alloc_count);
	}

	void *dest = vec_at(vec, vec->count);
	memcpy(dest, val, vec->_type_size);
	vec->count++;
	return dest;
}

void *vec_at(vector *vec, uint32_t index) {
	return vec->raw_array + vec->_type_size * index;
}

void vec_free(vector *vec) {
	if (vec->raw_array != NULL) {
		free(vec->raw_array);
	}

	free(vec);
}
