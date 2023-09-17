/*

    Extendible Hash Table (EHT) Implementation
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

#include "eht.h"

template<typename key, typename value>
using extendible_hash = ExtendibleHashTable<key, value>;

/*

    This is an implementation of an Extendible Hash Table (EHT), a dynamic
    data structure used for efficient key-value storage and retrieval.
    The EHT allows for dynamic resizing and distribution of data across
    multiple buckets, ensuring efficient access times and scalability.

    Some basic rules about Buckets:
        - Maximum depth of 50 (EHT_MAX_BUCKET_DEPTH)
        - Maximum of 50 pairs (EHT_MAX_BUCKET_SIZE)
        - Local depth (bucket depth) cannot exceed global depth (table depth)
        - At least 1 key-value pair

*/

struct Page {
    uint32_t data;
}; // Dummy structure for Page

template<typename k, typename v>
size_t extendible_hash<k, v>::getLocalDepth(bkt_id_t bktId) const {
    if (directory[bktId]) {
        return directory[bktId]->depth;
    }
    return INVALID_ID; // Bucket doesn't exist
}

template<typename k, typename v>
size_t extendible_hash<k, v>::getGlobalDepth() const {
    return depth;
}

template<typename k, typename v>
size_t extendible_hash<k, v>::getNumBuckets() const {
    return nBkt;
}

/* Use the page ID prefix bits to find its corresponding bkt_id_t */
template<typename k, typename v>
bkt_id_t extendible_hash<k, v>::bktIndex(const k& key) {
    return static_cast<bkt_id_t>((hashKey(key)) & ((1 << depth) - 1));
}

/* Use C++ built-in hash structure to hash a key */
template<typename k, typename v>
size_t extendible_hash<k, v>::hashKey(const k& key) {
    return std::hash<k>()(key);
}

/* Lookup (and grab) */
template<typename k, typename v>
EHTStatus extendible_hash<k, v>::grab(const k& key, v& val) {

    std::lock_guard<std::mutex> lock(latch);

    bkt_id_t index = bktIndex(key);

    if (!directory[index]) {
        return EHTStatus::INDEX_OUT_OF_BOUNDS;
    }

    /* Get bucket pairs */
    auto& items = directory[index]->items;
    auto item = directory[index]->items.find(key);

    /* Find the pair and place the value pointer in `val`  */
    if (item != items.end()) {
        val = item->second;
        return EHTStatus::GENERAL_SUCCESS;
    }
    return EHTStatus::NONEXISTENT_KEY;
}

/* Deletion */
template<typename k, typename v>
EHTStatus extendible_hash<k, v>::del(const k& key) {

    std::lock_guard<std::mutex> lock(latch);

    bkt_id_t index = bktIndex(key);

    if (!directory[index]) {
        return EHTStatus::INDEX_OUT_OF_BOUNDS;
    }

    auto& items = directory[index]->items;
    auto item = items.find(key);

    if (item != items.end()) {
        items.erase(item);
        --nPairs;
        return EHTStatus::GENERAL_SUCCESS;
    }
    return EHTStatus::NONEXISTENT_KEY;
}

/* Overflow into child bucket */
template<typename k, typename v>
std::unique_ptr<typename ExtendibleHashTable<k, v>::Bucket> extendible_hash<k, v>::\
split(std::shared_ptr<Bucket>& bkt) {

    std::lock_guard<std::mutex> lock(latch);

    /* Child to propagate from bkt */
    std::unique_ptr<Bucket> childBkt = std::make_unique<Bucket>(0, bkt->depth);

    while (childBkt->items.empty()) {
        ++bkt->depth;
        ++childBkt->depth;

        /* Propagate from parent to child */
        for (auto& item : bkt->items) {

            bkt_id_t hash = hashKey(item.first);

            if (hash & (1 << (bkt->depth - 1))) {
                childBkt->items.insert(item);
                childBkt->id = (hash & ((1 << bkt->depth) - 1));
            }
            else continue;

        }

        /* Bucket has no pairs? */
        if (bkt->items.empty()) {
            std::swap(bkt->items, childBkt->items);
            bkt->id = childBkt->id;
        }

        /* All keys have the same hash value? */
        if (bkt->depth == EHT_MAX_BUCKET_DEPTH) {
            bkt->overflowed = true;
            return nullptr;
        }
    }

    ++nBkt;
    return childBkt;
}

/* Put implementation

Handles overflows by splitting and propagating a bucket.
The method will adjust the global depth as needed.

*/
template<typename k, typename v>
EHTStatus extendible_hash<k, v>::put(const k& key, const v& val) {

    std::lock_guard<std::mutex> lock(latch);
    bkt_id_t bktId = bktIndex(key);

    if (!directory[bktId]) {
        directory[bktId] = std::make_shared<Bucket>(bktId, depth);
        ++nBkt;
    }
    auto bkt = directory[bktId];

    if (bkt->items.find(key) != bkt->items.end()) {
        bkt->items[key] = val;
        return EHTStatus::GENERAL_SUCCESS;
    }
  
    bkt->items.insert({ key, val });
    ++nPairs;

    if (bkt->items.size() > EHT_MAX_BUCKET_SIZE && !bkt->overflowed) {

        bkt_id_t idBc = bkt->id;        // id before changes
        size_t depthBc = bkt->depth;    // depth before changes
        std::shared_ptr<Bucket> childBkt = split(bkt);

        /* Restore depth if overflowed bucket can't be split */
        if (!childBkt) {
            bkt->depth = depthBc;
            return EHTStatus::GENERAL_SUCCESS;
        }

        if (bkt->depth > depth) {

            size_t size = directory.size();
            size_t factor = (1 << (bkt->depth - depth));

            /* Maintaining depth rules */
            depth = bkt->depth;
            directory.resize(directory.size() * factor);
            directory[bkt->id] = bkt;
            directory[childBkt->id] = childBkt;

            /* Ensure integrity of bucket pointers */
            for (size_t i = 0; i < size; ++i) {
                if (directory[i]) {

                    /* Check for stale relationship: Mismatched bucket and directory prefix bits */
                    if (i < directory[i]->id ||
                        (i & (1 << directory[i]->depth - 1) != directory[i]->id)) {
                        directory[i].reset();
                    }

                    /* Update directory entries to maintain integrity */
                    else {
                        size_t step = 1 << directory[i]->depth;
                        for (size_t j = i + step; j < directory.size(); j += step) {
                            directory[j] = directory[i];
                        }
                    }
                }
                else {

                    for (size_t i = idBc; i < directory.size(); i += (1 << depthBc)) {
                        directory[i].reset();
                    }

                    /* Add new buckets to the directory */
                    directory[bkt->id] = bkt;
                    directory[childBkt->id] = childBkt;
                    size_t step = 1 << directory[i]->depth;

                    for (size_t i = bkt->id + step; i < directory.size(); i += step) {
                        directory[i] = bkt;
                    }

                    for (size_t i = childBkt->id + step; i < directory.size(); i += step) {
                        directory[i] = childBkt;
                    }

                }
            } // for (size_t i = 0; i < size; ++i)
        } // if (bkt->depth > depth)
    } // if (bkt->items.size() > EHT_BUCKET_SIZE && !bkt->overflowed)

    return EHTStatus::GENERAL_SUCCESS;
}

/* Production */
template class ExtendibleHashTable<page_id_t, Page*>;
template class ExtendibleHashTable<Page*, std::list<Page*>::iterator>;

/* Test */
template class ExtendibleHashTable<int, std::string>;
template class ExtendibleHashTable<int, int>;
template class ExtendibleHashTable<int, char>;