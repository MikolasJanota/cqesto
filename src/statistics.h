/*
 * File:  statistics.h
 * Author:  mikolas
 * Created on:  Fri Dec 23 12:47:28 CET 2022
 * Copyright (C) 2022, Mikolas Janota
 */
#pragma once
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class StatisticsManager {
  public:
    StatisticsManager() {
        all.push_back(bts = new IntStatistic("Backtracks"));
        all.push_back(satCalls = new IntStatistic("SAT calls"));
        all.push_back(satTime = new DoubleStatistic("SAT time"));
        all.push_back(totalTime = new DoubleStatistic("Total time"));
    }

    virtual ~StatisticsManager();
    class Statistic {
      public:
        Statistic(const std::string &name);
        virtual ~Statistic();

        const std::string &name() const { return d_name; }

        virtual std::ostream &print(std::ostream &) = 0;
        virtual bool should_print() const = 0;

      private:
        const std::string d_name;
    };

    class DoubleStatistic : public Statistic {
      public:
        DoubleStatistic(const std::string &name, double init_value = 0)
            : Statistic{name}, m_val{init_value} {};

        double inc(double ival) {
            d_print = true;
            return m_val += ival;
        }

        double get() const { return m_val; }

        virtual bool should_print() const override { return d_print; }

        virtual std::ostream &print(std::ostream &o) override {
            return o << name() << " : " << std::fixed << std::setprecision(3)
                     << m_val;
        }

      private:
        double m_val;
        bool d_print = false;
    };

    class IntStatistic : public Statistic {
      public:
        IntStatistic(const std::string &name, int init_value = 0)
            : Statistic{name}, m_val{init_value} {};
        int inc() { return ++m_val; }
        int get() const { return m_val; }
        void set(int v) { m_val = v; }

        virtual bool should_print() const override { return m_val != 0; }

        virtual std::ostream &print(std::ostream &o) override {
            return o << name() << " : " << m_val;
        }

      private:
        int m_val;
    };
    IntStatistic *satCalls;
    IntStatistic *bts;
    DoubleStatistic *satTime;
    DoubleStatistic *totalTime;
    std::vector<Statistic *> all;
};
