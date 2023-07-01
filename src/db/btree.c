#include "btree.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "table.h"

void leaf_init(btree_leaf *node, uint32_t page, uint32_t record_length);
uint32_t leaf_find_cell(btree_leaf *node, md5_t *key);
uint32_t inner_find_child(btree_inner *node, md5_t *key);
btree_leaf *find_leaf(db_table *table, btree_inner *start, md5_t *key);
btree_cursor *find_key(db_table *table, md5_t *key);

void btree_init(db_table *table, uint32_t record_length) {
	// Get or create page 0
	btree_leaf *root = table->cmeta.total_pages == 0 ?
		table_new_norm_page(table, NULL) :
		table_get_norm_page(table, 0);
	leaf_init(root, 0, record_length);
	table->cmeta.root_page = 0;
}

int btree_insert(db_table *table, md5_t *key, void *record) {
	// Get insert node
	btree_cursor *location = find_key(table, key);
	btree_leaf *target = table_get_norm_page(table, location->pg_value);

	// Node is full, must split
	if (target->cell_count == LEAF_DATA_MEM / target->record_length) {
		// TODO: implement splitting
	}

	// Inserting in the middle, move bigger elements
	if (location->cell_num < target->cell_count) {
		for (uint32_t i = target->cell_count; i > location->cell_num; i--) {
			void *dest = target->records + target->record_length * i;
			void *src = target->records + target->record_length * (i - 1);
			memcpy(dest, src, target->record_length);
		}
	}

	// Insert key
	uint8_t *record_ptr = target->records + target->record_length * location->cell_num;
	md5_cp((md5_t *)record_ptr, key);
	// Insert row
	record_ptr += sizeof(md5_t);
	memcpy(record_ptr, record, target->record_length);

	target->cell_count++;

	return 0;
}

/** Private functions */

/**
 * @brief Initialize leaf node.
 *
 * @param[in] node - Leaf node location.
 * @param[in] page - Node location in database.
 * @param[in] record_length - Length of each record in leaf (include md5 hash key).
 */
void leaf_init(btree_leaf *node, uint32_t page, uint32_t record_length) {
	// Write header
	node->header.type = NODE_LEAF;
	node->header.is_root = 1;
	node->header.pg_self = page;
	node->header.pg_parent = INVALID_VAL;

	// Leaf-specific
	node->next_leaf = INVALID_VAL;
	node->cell_count = 0;
	node->record_length = record_length;
}

/**
 * @brief Find position or best placement of key inside leaf node.
 *
 * @param[in] node - Leaf node object.
 * @param[in] key - Hash key pointer.
 * @return Cell index inside leaf node.
 */
uint32_t leaf_find_cell(btree_leaf *node, md5_t *key) {
	uint32_t min = 0;
	uint32_t max = node->cell_count;

	// Do binary search
	while (min != max) {
		uint32_t cell = (min + max) / 2;
		md5_t *cell_key = (md5_t *)(node->records + (node->record_length * cell));

		if (md5_eq(*key, *cell_key)) {
			return cell;
		}

		if (md5_ls(*key, *cell_key)) {
			max = cell;
		} else {
			min = cell + 1;
		}
	}

	return min;
}

/**
 * @brief Find child for key inside an inner node.
 *
 * @param[in] node - Inner node object.
 * @param[in] key - Hash key pointer.
 * @return Child index inside the inner node.
 */
uint32_t inner_find_child(btree_inner *node, md5_t *key) {
	uint32_t min = 0;
	uint32_t max = node->child_count;

	// Binary search
	while (min != max) {
		uint32_t child = (min + max) / 2;
		md5_t *child_key = &node->children[child].key;

		if (md5_eq(*key, *child_key)) {
			return child;
		}

		if (md5_ls(*key, *child_key)) {
			max = child;
		} else {
			min = child + 1;
		}
	}

	return min;
}

/**
 * @brief Find leaf for key inside inner node.
 *
 * @param[in] table - Table to search.
 * @param[in] start - Search start node.
 * @param[in] key - Hash key pointer.
 * @return Leaf node best fit for key.
 */
btree_leaf *find_leaf(db_table *table, btree_inner *start, md5_t *key) {
	// Binary search for child
	uint32_t index = inner_find_child(start, key);
	page_t pg_child = (index == start->child_count) ? start->right_child : start->children[index].pg_child;
	btree_header *child = table_get_norm_page(table, pg_child);

	// Recurse if another inner node found
	if (child->type == NODE_LEAF) {
		return (btree_leaf *)child;
	} else {
		return find_leaf(table, (btree_inner *)child, key);
	}
}

/**
 * @brief Find key position or best placement.
 *
 * @param[in] table - Table object.
 * @param[in] key - Hash key pointer.
 * @return Cursor object pointing to location.
 */
btree_cursor *find_key(db_table *table, md5_t *key) {
	btree_cursor *cur = malloc(sizeof(btree_cursor));
	cur->table = table;

	btree_header *root_header = table_get_norm_page(table, table->cmeta.root_page);
	btree_leaf *target = (root_header->type == NODE_LEAF) ?
		(btree_leaf *)root_header :
		find_leaf(table, (btree_inner *)root_header, key);

	cur->pg_value = target->header.pg_self;
	cur->cell_num = leaf_find_cell(target, key);
	cur->end = (cur->cell_num == target->cell_count && target->next_leaf == INVALID_VAL);

	return cur;
}
