#include "btree.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "table.h"

#define leaf_max_cells(leaf_ptr) (LEAF_DATA_MEM / leaf_ptr->record_length)
#define leaf_cell_at(leaf_ptr, i) (leaf_ptr->records + leaf_ptr->record_length * i)
#define leaf_cell_body_at(leaf_ptr, i) (leaf_cell_at(leaf_ptr, i) + sizeof(md5_t))

void leaf_init(btree_leaf *node, page_t page, uint32_t record_length);
void inner_init(btree_inner *node, page_t page);
uint32_t leaf_find_cell(btree_leaf *node, md5_t *key);
uint32_t inner_find_child(btree_inner *node, md5_t *key);
btree_leaf *find_leaf(db_table *table, btree_inner *start, md5_t *key);
btree_cursor *find_key(db_table *table, md5_t *key);
int leaf_split_insert(btree_leaf *old_node, md5_t *key, void *record, btree_cursor *location);
md5_t *max_key(db_table *table, btree_header *node);

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
	if (target->cell_count == leaf_max_cells(target)) {
		return leaf_split_insert(target, key, record, location);
	}

	// Inserting in the middle, move bigger elements
	if (location->cell_num < target->cell_count) {
		for (uint32_t i = target->cell_count; i > location->cell_num; i--) {
			void *dest = leaf_cell_at(target, i);
			void *src = leaf_cell_at(target,  i - 1);
			memcpy(dest, src, target->record_length);
		}
	}

	// Insert record
	md5_cp((md5_t *)leaf_cell_at(target, location->cell_num), key);
	memcpy(leaf_cell_body_at(target, location->cell_num), record, target->record_length);

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
void leaf_init(btree_leaf *node, page_t page, uint32_t record_length) {
	// Write header
	node->header.type = NODE_LEAF;
	node->header.is_root = 1;
	node->header.pg_self = page;
	node->header.pg_parent = INVALID_VAL;

	// Leaf-specific
	node->pg_next_leaf = INVALID_VAL;
	node->cell_count = 0;
	node->record_length = record_length;
}

/**
 * @brief Initialize inner node.
 *
 * @param[in] node - Inner node location.
 * @param[in] page - Node location in database.
 */
void inner_init(btree_inner *node, page_t page) {
	// Write header
	node->header.type = NODE_INNER;
	node->header.is_root = 1;
	node->header.pg_self = page;
	node->header.pg_parent = INVALID_VAL;

	// Inner-specific
	node->pg_right_child = INVALID_VAL;
	node->child_count = 0;
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
	page_t pg_child = (index == start->child_count) ? start->pg_right_child : start->children[index].pg_child;
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
	cur->end = (cur->cell_num == target->cell_count && target->pg_next_leaf == INVALID_VAL);

	return cur;
}

/**
 * @brief Split the leaf node and insert record at desired location.
 *
 * @param[in] old_node - Old (target) leaf node.
 * @param[in] key - Hash key pointer.
 * @param[in] record - Data record to insert.
 * @param[in] location - Location to place the record.
 * @return Success code.
 */
int leaf_split_insert(btree_leaf *old_node, md5_t *key, void *record, btree_cursor *location) {
	// Create new node
	page_t page_buf;
	btree_leaf *new_node = (btree_leaf *)table_new_norm_page(location->table, &page_buf);
	leaf_init(new_node, page_buf, old_node->record_length);

	// Calculate split sizes
	uint32_t split_left = (leaf_max_cells(old_node) + 1) / 2;
	uint32_t split_right = (leaf_max_cells(old_node) + 1) - split_left;

	// Split items between pages
	for (uint32_t i = 0; i <= leaf_max_cells(old_node); i++) {
		btree_leaf *target_node = (i >= split_left) ? new_node : old_node;
		void *dest = leaf_cell_at(target_node, i);

		if (i == location->cell_num) {
			md5_cp((md5_t *)dest, key);
			memcpy(leaf_cell_body_at(target_node, i), record, target_node->record_length);
			continue;
		}

		void *src = (i > location->cell_num) ?
			leaf_cell_at(old_node, i - 1) :
			leaf_cell_at(old_node, i);

		memcpy(dest, src, target_node->record_length);
	}

	// Update counts
	old_node->cell_count = split_left;
	new_node->cell_count = split_right;

	// Attach to parent
	if (old_node->header.is_root) {
		// Create new root node
		btree_inner *new_root = table_new_norm_page(location->table, &page_buf);
		inner_init(new_root, page_buf);
		location->table->cmeta.root_page = page_buf;

		// Attach children
		new_root->child_count = 1;
		md5_cp(&new_root->children[0].key, max_key(location->table, (btree_header *)old_node));
		new_root->children[0].pg_child = old_node->header.pg_self;
		new_root->pg_right_child = new_node->header.pg_self;
	} else {
		page_buf = old_node->header.pg_parent;
		btree_inner *parent = table_get_norm_page(location->table, page_buf);
		// TODO: update old node
		// TODO: internal insert
	}

	// De-root nodes
	old_node->header.is_root = 0;
	old_node->header.pg_parent = page_buf;
	new_node->header.is_root = 0;
	new_node->header.pg_parent = page_buf;

	return 0;
}

/**
 * @brief Get max key inside a node.
 *
 * @param[in] table - Table object.
 * @param[in] node - Node to search.
 * @return Hash key pointer to max key.
 */
md5_t *max_key(db_table *table, btree_header *node) {
	if (node->type == NODE_LEAF) {
		btree_leaf *full_node = (btree_leaf *)node;
		return (md5_t *)leaf_cell_at(full_node, full_node->cell_count - 1);
	} else {
		btree_inner *full_node = (btree_inner *)node;
		return max_key(table, table_get_norm_page(table, full_node->pg_right_child));
	}
}
