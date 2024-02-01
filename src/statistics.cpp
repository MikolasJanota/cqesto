/*
 * File:  statistics.cpp
 * Author:  mikolas
 * Created on:  Fri Dec 23 12:53:38 CET 2022
 * Copyright (C) 2022, Mikolas Janota
 */
#include "statistics.h"

StatisticsManager::~StatisticsManager() {
    for (auto p : all)
        delete p;
}

StatisticsManager::Statistic::Statistic(const std::string &name)
    : d_name(name){};
StatisticsManager::Statistic::~Statistic() {}
