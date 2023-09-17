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

#include "config.h"

// Go through these includes later
#include <string>
#include <list>
#include <mutex>
#include <vector>
#include <map>

/*

    Extendible Hash Tables (EHTs) are a data structure used in database
    systems. They use dynamic hashing to store directory-bucket pairs
    for efficient page access.

       Directories (hash)       Buckets (key k, value v)

           |==========|            |============|
           |    01     ============> Key->Value |
           |==========|            |============|
           |    10     ============> Key->Value |
           |==========|            |============|
           |    00     ============> Key->Value |
           |==========|            |============|
           |    11     ============> Key->Value |
           |==========|            |============|

    To access Pages, you search the table with a page_id_t (Page ID).

*/

/* Status codes */
enum class EHTStatus {
    GENERAL_SUCCESS,
    GENERAL_FAILURE,
    INDEX_OUT_OF_BOUNDS,
    NONEXISTENT_KEY
};

/* Key-value template class for EHT */
template <typename k, typename v>
class ExtendibleHashTable {

    struct Bucket {
        Bucket() = default;
        Bucket(bkt_id_t bktId, size_t bktDepth) : id(bktId), depth(bktDepth) {}

        bool overflowed = false;    // Bucket overflow
        std::map<k, v> items;       // Key-value pairs
        bkt_id_t id = 0;            // Identifier
        size_t depth = 0;           // Local depth
    };

public:

    ExtendibleHashTable(size_t size) :
        nBkt(1), nPairs(0), depth(0) {
        directory.emplace_back(new Bucket(0, 0));
    }

    size_t getGlobalDepth() const;
    size_t getLocalDepth(bkt_id_t bucket_id) const;
    size_t getNumBuckets() const;

    EHTStatus grab(const k& key, v& val);
    EHTStatus del(const k& key);
    EHTStatus put(const k& key, const v& val);
    size_t size() const { return nPairs; }

private:

    std::mutex latch;           // Mutex for concurrency
    size_t nBkt;              // Number of active buckets
    size_t depth;               // Global depth
    size_t nPairs;              // Key-value pair count in the table
    std::vector<std::shared_ptr<Bucket>> directory; // Smart ptr directory->bucket

    std::unique_ptr<Bucket> split(std::shared_ptr<Bucket>&);
    size_t hashKey(const k& key);
    bkt_id_t bktIndex(const k& key);

};

#endif
