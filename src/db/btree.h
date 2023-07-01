#pragma once

#include <stdint.h>

#include "defines.h"
#include "table.h"

// Header: node type
typedef enum {
	NODE_INNER,
	NODE_LEAF
} btree_node_type;

// Node header
typedef struct {
	btree_node_type type;
	uint8_t is_root;
	page_t pg_self;
	page_t pg_parent;
} btree_header;

/** Inner/Internal Nodes */

// Inner: child node description
typedef struct {
	page_t pg_child;
	md5_t key;
} btree_inner_child;

#define INNER_DATA_MEM (PAGE_SIZE - sizeof(btree_header) - sizeof(page_t) - sizeof(uint32_t))
#define INNER_KEYS (INNER_DATA_MEM / sizeof(btree_inner_child))
#define INNER_PADDING (INNER_DATA_MEM % sizeof(btree_inner_child))

// Inner node, sizeof = PAGE_SIZE
typedef struct {
	btree_header header;
	page_t right_child;
	// Excluding right child
	uint32_t child_count;
	btree_inner_child children[INNER_KEYS];
	uint8_t _padding[INNER_PADDING];
} btree_inner;

/** Leaf/Data Nodes */

#define LEAF_DATA_MEM (PAGE_SIZE - sizeof(btree_header) - sizeof(page_t) - sizeof(uint32_t) * 2)

// Leaf node, sizeof = PAGE_SIZE
typedef struct {
	btree_header header;
	page_t next_leaf;
	uint32_t cell_count;
	uint32_t record_length;
	uint8_t records[LEAF_DATA_MEM];
} btree_leaf;

/** Cursors */

typedef struct {
	db_table *table;
	page_t pg_value;
	uint32_t cell_num;
	uint8_t end;
} btree_cursor;

/**
 * @brief Initialize empty database btree.
 *
 * @param[in] table - Table object.
 * @param[in] record_length - Length of individual record.
 */
void btree_init(db_table *table, uint32_t record_length);

/**
 * @brief Insert record into the database btree.
 *
 * @param[in] table - Table object.
 * @param[in] key - Hash key pointer to insert.
 * @param[in] record - Data record to insert (excluding key).
 * @return Status code.
 */
int btree_insert(db_table *table, md5_t *key, void *record);
