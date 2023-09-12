/*

    Copyright 2023 Marcus Antonelli

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at:

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef HERACLES_CONFIG_H
#define HERACLES_CONFIG_H

#include <stdint.h>

/* ID */
#define  INVALID_ID             -1        // Invalid ID for page, lsn, trx, and bkt 
#define  HEADER_PAGE_ID         0         // Fixed ID for header page

/* Page */
#define  PAGE_SIZE              4096      // Common page size, can be changed to any base 2 
#define  LRU_CACHE_LIMIT        2000      // Max number of pages at a given time in LRU cache

/* WAL */
#define  LOGGING_ENABLED        1         // Default true, in rare instances may need to be false
#define  LOG_BUFFER_SIZE        65536     // Journal size of 64 kb

/* Limits */
#define  EHT_BUCKET_SIZE        50        // Number of key-value pairs in a given EHT bucket
#define  BPTREE_MAX_HEIGHT      20        // Maximum depth/height of a B+ tree
#define  DB_MAX_PAGES           0         // Maximum number of pages in a database

/* Queries */
#define  SQL_MAX_LENGTH         1000000   // Max length of an SQL query
#define  SQL_MAX_TABLE_JOIN     64        // Max number of tables in a join
#define  ETREE_MAX_HEIGHT       1000      // Max height for SQL expression tree

/*

Define later:

#define  TABLE_MAX_PAGES        x   
#define  TABLE_MAX_ENTRIES      x  
#define  TABLE_MAX_COLS         x
#define  COLUMNS_PER_PAGE       x

*/

typedef int32_t page_id_t;  // Page id type def
typedef int32_t trx_id_t;   // Transaction id type def
typedef int32_t lsn_t;      // Log sequence number type def
typedef int32_t bkt_id_t;   // Hash bucket id type def

#endif