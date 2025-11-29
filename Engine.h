#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>   
#include <vector>     
#include "BST.h"      
#include "Record.h"
//add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine {
    vector<Record> heap;                  // the main data store (simulates a heap file)
    BST<int, int> idIndex;                // index by student ID
    BST<string, vector<int>> lastIndex;   // index by last name (can have duplicates)

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record &recIn) {
        int recordIndex = heap.size();
        heap.push_back(recIn);

        idIndex.insert(recIn.id, recordIndex);

        string lastNameKey = toLower (recIn.last);

        auto vectorPointer = lastIndex.find(lastNameKey);
        if (vectorPointer) {
            vectorPointer->push_back(recordIndex);
        } else {
            lastIndex.insert (lastNameKey, vector<int>{recordIndex});
        }
        return recIn.id;
    }

    //If entry for lowercase last name exists then append to vector, otherwise
    //create new vector and insert into bst

    //Used auto to have compiler automatically determine variable type

    

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id) {
        //idIndex.resetMetrics();

        int *recordIndexPointer = idIndex.find(id);

        if (!recordIndexPointer) {
            return false;
        }

        int recordIndex = *recordIndexPointer;

        if (recordIndex < 0 || recordIndex >= (int)heap.size()) {
            return false;
        }

        heap[recordIndex].deleted = true;

        idIndex.erase(id);

        return true;
    }

    //If record is out of range don''t delete. Otherwise mark record as
    //logically deleted in the heap. Remove ID from ID index.

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record *findById(int id, int &cmpOut) {
        idIndex.resetMetrics();

        int *recordIndexPointer = idIndex.find(id);

        cmpOut = idIndex.comparisons;

        if (!recordIndexPointer) {
            return nullptr;
        }

        int recordIndex = *recordIndexPointer;

        if (recordIndex < 0 || recordIndex >= (int)heap.size()) {
            return nullptr;
        }

        if (heap[recordIndex].deleted) {
            return nullptr;
        }

        return &heap[recordIndex];
    }

    //Confirm record index is within bounds of heap.
    //If record is out of range or record is marked deleted
    //then search fails and returns null pointer. Otherwise
    //return pointer to the non-deleted record

    // Returns all records with ID in the range [lo, hi].
    // Also reports the number of key comparisons performed.
    vector<const Record *> rangeById(int lo, int hi, int &cmpOut) {
        vector<const Record *> results;

        idIndex.resetMetrics();

        idIndex.rangeApply(lo, hi, [&](const int &key, int &recordIndex) {
            if (recordIndex >= 0 &&
                recordIndex < (int)heap.size() &&
                !heap[recordIndex].deleted) {
                    results.push_back(&heap[recordIndex]);
                }
        }
    );

        cmpOut = idIndex.comparisons;

        return results;
    }

    //Filters out invalid indexes and records marked as deleted

    // Returns all records whose last name begins with a given prefix.
    // Case-insensitive using lowercase comparison.
    vector<const Record *> prefixByLast(const string &prefix, int &cmpOut) {
        vector<const Record *> results;

        string lowerPrefix = toLower(prefix);

        string upperBound = lowerPrefix + "\xFF";

        lastIndex.resetMetrics();

        lastIndex.rangeApply(lowerPrefix, upperBound, 
            [&](const string &lastNameKey, vector<int> &recordIndices) {
                
                if (lastNameKey.rfind(lowerPrefix, 0) == 0) {
                    for (int recordIndex : recordIndices) {
                        if (recordIndex >= 0 &&
                            recordIndex < (int)heap.size() &&
                            !heap[recordIndex].deleted) {

                                results.push_back(&heap[recordIndex]);
                            }
                    }
                }
            }

            );

            cmpOut = lastIndex.comparisons;

            return results;
        }

        //Used a computed upper bound to limit the scan to keys that
        //share the prefix, and verifies each result is valid and not deleted

    };
    

#endif
