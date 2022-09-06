#pragma once

struct RelativeIndex{
    size_t doc_id;
    float rank;

    RelativeIndex(): doc_id(0), rank(0) {};
    RelativeIndex(size_t _doc_id, float _rank): doc_id(_doc_id), rank(_rank) {};

    bool operator==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};