/*
 * File:  table.h
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 4:52:18 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#pragma once
#include <map>
#include <cstddef>
#include <unordered_map>
#include <vector>
template <class ContentType, class ContentHash, class ContentEqual>
class Table {
  public:
    Table() : entry_count(0) {}
    ContentType get(size_t index) const { return s[index]; }
    size_t lookup(ContentType content) {
        const auto it = m.find(content);
        if (it == m.end()) {
            s.resize(entry_count + 1);
            s[entry_count] = content;
            m[content] = entry_count;
            return entry_count++;
        } else {
            return it->second;
        }
    }

  private:
    size_t entry_count;
    std::vector<ContentType> s;
    std::unordered_map<ContentType, size_t, ContentHash, ContentEqual> m;
};
