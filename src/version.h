/*
 * File:  version.h
 * Author:  mikolas
 * Created on:  Tue Aug 10 13:57:40 CEST 2021
 * Copyright (C) 2021, Mikolas Janota
 */
#pragma once
#include <string>

struct Version {
    static const std::string GIT_SHA1;
    static const std::string GIT_DATE;
    static const std::string GIT_COMMIT_SUBJECT;
};
