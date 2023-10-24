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
#include <stdlib.h>
#include <memory>
#include <vector>
#include "os_ppd.h"

/* ID */
#define  INVALID_ID             -1        // Invalid ID for page, lsn, trx, and bkt 
#define  HEADER_PAGE_ID         0         // Fixed ID for header page

/* Page */
#define  PAGE_SIZE              4096      // Common page size, can be changed to any base 2 
#define  LFU_CACHE_LIMIT        750       // Max number of unprivileged frames in cache
#define  LRU_CACHE_LIMIT        1250      // Max number of privileged frames in cache
#define  LAZY_DECOMPRESSION     0         // Decompress data when loading into memory, or when reading?
#define  LFRU_CACHE_LIMIT       LFU_CACHE_LIMIT + LRU_CACHE_LIMIT

/* WAL */
#define  LOGGING_ENABLED        1         // Default true, in rare instances may need to be false
#define  LOG_BUFFER_SIZE        65536     // Journal size of 64 kb

/* Limits */
#define  EHT_MAX_BUCKET_DEPTH   50        // Maximum depth of a single EHT bucket 
#define  EHT_MAX_BUCKET_SIZE    50        // Number of key-value pairs in a given EHT bucket
#define  BPTREE_MAX_HEIGHT      20        // Maximum depth/height of a B+ tree
#define  DB_MAX_PAGES           0         // Maximum number of pages in a database

/* Queries */
#define  SQL_MAX_LENGTH         1000000   // Max length of an SQL query
#define  SQL_MAX_TABLE_JOIN     64        // Max number of tables in a join
#define  ETREE_MAX_HEIGHT       1000      // Max height for SQL expression tree

/* System */
#define  DISK_LIMIT             30        // Measured as 2^N bytes

#if defined(_M_AMD64) || defined(__x86_64__) || defined(__arm64__)
#   define CRC_POLYNOMIAL       0xEDB88320          // Standard 32-bit polynomial
#   define SIZE_64              1 
    typedef int64_t bit_ofst;   // Bit offset
#else
#   define CRC_POLYNOMIAL       0x42F0E1EBA9EA3693  // Standard 64-bit polynomial
#   define SIZE_64              0
    typedef int32_t bit_ofst;
#endif

template <typename T>
using bitset = std::vector<T>; 

typedef int64_t page_id_t;  // Page id type def
typedef int32_t txn_id_t;   // Transaction id type def
typedef int32_t log_id_t;   // Log sequence number (LSN)
typedef int16_t bkt_id_t;   // Hash bucket id
typedef int32_t col_id_t;   // Column id 

#endif