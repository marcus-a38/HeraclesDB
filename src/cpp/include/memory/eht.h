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

#ifndef HERACLES_EXT_HASH_H
#define HERACLES_EXT_HASH_H

#include <map>
#include <memory>
#include <vector>
#include "src/cpp/include/general/config.h"

/* 

    Extendible Hash Tables (EHTs) are a data structure used in database 
    systems. They use dynamic hashing to store directory-bucket pairs 
    for efficient page access.

    Basic structure:
                                                                
       Directories (hash, key k)       Buckets (value v)       

           |==========|            |=======================|    
           |    01     ============> Page memory address 1      
           |==========|            |=======================|    
           |    10     ============> Page memory address 2      
           |==========|            |=======================|    
           |    00     ============> Page memory address 3      
           |==========|            |=======================|    
           |    11     ============> Page memory address N      
           |==========|            |=======================|    
 
    To access data, you search the table with a page_id_t (Page ID)

*/

enum class EHTStatus {
    GENERAL_SUCCESS,        // Catch-all for other general successes
    GENERAL_FAILURE,        // Catch-all for other general errors
    NEW_BKT_SUCCESS,        // Creating bucket succeeded
    NEW_BKT_FAILURE,        // Creating bucket failed
    OVERFLOW_BKT_SUCCESS,   // Bucket overflow routine succeeded
    OVERFLOW_BKT_FAILURE,   // Bucket overflow routine failed
    INDEX_OUT_OF_BOUNDS,    // Invalid bucket index
    MEMORY_EXCEPTION        // Any memory-related error
};

/* Key-value template class for EHT */
template <typename k, typename v> 
class ExtendibleHashTable {

    struct Bucket {

        bool isOverflowed = false;  // Bucket overflow
        std::map<k, v> items;       // Key-value pairs
        bkt_id_t id = 0;            // Identifier
        uint32_t depth = 0;         // Local depth

        Bucket() = default;
        Bucket(bkt_id_t bid, int bdepth): id(bid), depth(bdepth) {}

    };

    private:

        const size_t bktSize;       // Max (n) number of elements in a bucket
        uint32_t bktCount;          // Number of active buckets
        uint32_t depth;             // Global depth
        size_t numPairs;            // Key-value pair count in the table
        std::vector<std::shared_ptr<Bucket>> directory; // Smart ptr directory->bucket

        std::unique_ptr<Bucket> split(std::shared_ptr<Bucket>&);
        size_t hashKey(const k &key);
        size_t bktIndex(const k &key);
    
    public:

        int getGlobalDepth() const;
        int getLocalDepth(bkt_id_t bucket_id) const;
        int getNumBuckets() const;
        
        bool search(const k &key, v &val);
        bool remove(const k &key);
        void insert(const k &key, const v &val);
        size_t size() const { return numPairs; }

        ExtendibleHashTable(size_t size):
            bktSize(size), bktCount(1), depth(0), numPairs(0) {
                directory.emplace_back(new Bucket(0, 0));
        }
};

/* Using a more palatable name */
template<typename key, typename value>
using extendible_hash = ExtendibleHashTable<key, value>;

#endif