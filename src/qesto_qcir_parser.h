/*
 * File:  qesto_qcir_parser.h
 * Author:  mikolas
 * Created on:  Fri Jul 28 14:29:19 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */
#pragma once
#include "auxiliary.h"
#include "data_structures.h"
#include "expressions.h"
#include "idvector.h"
#include "qcir_parser.h"
#include "qtypes.h"
#include "variable_manager.h"
#include "visitor.h"
#include <unordered_map>
namespace qesto {
class QestoQCIRParser : public QCIRParser {
  public:
    QestoQCIRParser(StreamBuffer &buf, Expressions &factory)
        : QCIRParser(buf), m_factory(factory) {}

    QFla formula() { return m_qfla; }
    bool has_free() const { return m_has_free; }

    const VariableManager &vmng() const { return m_vmng; }
    const std::unordered_map<std::string, int> &name2var() {
        return m_name2var;
    }

  protected:
    virtual void cb_qblock_quant(QType qt) override { m_qt = qt; }

    virtual void cb_qblock_var(std::string v) override {
        m_qcir_var_stack.push_back(make_varid(v));
    }

    virtual void cb_gate_stmt_gt(GType gt) override { m_gate_type = gt; }

    virtual void cb_gate_stmt_var(const std::string &v) override {
        m_gate_var = v;
    }

    virtual void cb_gate_stmt_lit(const Lit &l) override {
        m_qcir_ID_stack.push_back(get_ID(l));
    }

    virtual void cb_output_lit(const Lit &l) override { m_output_lit = l; }

    virtual void cb_gate_closed() override {
        define_gate(m_gate_var, make_gate());
        m_qcir_ID_stack.clear();
    }

    ID make_gate() {
        const auto &args = m_qcir_ID_stack;
        auto &f = m_factory;
        switch (m_gate_type) {
        case GType::AND: return f.make_and(IDVector(args));
        case GType::OR: return f.make_or(IDVector(args));
        case GType::XOR:
            if (m_qcir_ID_stack.size() != 2)
                semantic_error("currently only supporting binary XOR");
            return f.make_and(
                f.make_or(args[0], args[1]),
                f.make_or(f.make_not(args[0]), f.make_not(args[1])));
        case GType::ITE:
            if (m_qcir_ID_stack.size() != 3)
                semantic_error("ITE must have 3 arguments");
            return f.make_or(f.make_and(args[0], args[1]),
                             f.make_and(f.make_not(args[0]), args[2]));
        }
        assert(false);
        exit(1);
    }

    virtual void cb_quant_closed() override {
        switch (m_qt) {
        case QType::FREE:
            m_qfla.free = std::move(m_qcir_var_stack);
            m_has_free = true;
            break;
        case QType::EXIST:
            m_qfla.pref.push_back(qesto::Quantification(
                qesto::EXISTENTIAL, std::move(m_qcir_var_stack)));
            break;
        case QType::FORALL:
            m_qfla.pref.push_back(qesto::Quantification(
                qesto::UNIVERSAL, std::move(m_qcir_var_stack)));
            break;
        }
        m_qcir_var_stack.clear();
    }

    virtual void cb_file_closed() override {
        const auto &[sign, name] = m_output_lit;
        const auto i = m_name2id.find(name);
        if (i == m_name2id.end())
            semantic_error("output gate not defined '" + name + "'");
        m_qfla.output = sign ? m_factory.make_not(i->second) : i->second;
    }

  protected:
    Expressions &m_factory;
    VariableManager m_vmng;
    QFla m_qfla;
    bool m_has_free = false;
    Lit m_output_lit;
    std::string m_gate_var;
    GType m_gate_type;
    QType m_qt;
    std::vector<ID> m_qcir_ID_stack;
    std::vector<Var> m_qcir_var_stack;

    std::unordered_map<std::string, int> m_name2var;
    std::unordered_map<std::string, ID> m_name2id;

    ID get_ID(const QCIRParser::Lit &l) {
        const auto &[sign, name] = l;
        const auto i = m_name2id.find(name);

        if (i != m_name2id.end()) {
            const ID retv = i->second;
            return sign ? m_factory.make_not(retv) : retv;
        } else {
            const auto vi = m_name2var.find(name);
            if (vi == m_name2var.end())
                semantic_error("undefined variable or gate '" + name + "'");
            const Var v = vi->second;
            const ID retv = m_factory.make_lit(SATSPC::mkLit(v, sign));
            return retv;
        }
    }

    int make_varid(const std::string &var_name) {
        const auto retv = m_vmng.new_var();
        const auto i = m_name2var.insert({var_name, retv});
        if (!i.second)
            semantic_error("var redefinition '" + var_name + "'");
        return retv;
    }

    ID define_gate(const std::string &s, ID def) {
        const auto i = m_name2id.insert({s, def});
        if (!i.second)
            semantic_error("gate redefinition");
        return def;
    }

    void semantic_error(std::string msg);
};
} // namespace qesto
