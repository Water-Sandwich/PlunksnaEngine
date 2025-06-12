//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNALOG_H
#define PLUNKSNALOG_H

#include <iostream>

namespace Plunksna
{
enum Severity : unsigned char
{
    eINFO = 0,
    eWARNING,
    eLETHAL
};

#define RED "\033[49;31m"
#define YELLOW "\033[49;33m"
#define WHITE "\033[49;39m"
#define RESET "\033[0m"

#define INFO "[INFO]: "
#define WARNING "[WARNING]: "
#define LETHAL "[LETHAL]: "

consteval const char* getStr(Severity severity)
{
    switch (severity) {
    case Severity::eINFO:
        return WHITE INFO;
    case Severity::eWARNING:
        return YELLOW WARNING;
    case Severity::eLETHAL:
        return RED LETHAL;
    default:
        return RESET INFO;
    }
}

#define LOG_FILE __FILE__ << "." << __LINE__ << ": "

#ifdef __GNUC__
#define LOG_SIG __PRETTY_FUNCTION__ << ' '
#else
#define LOG_SIG typeid(*this).name() << "::" << __func__ << "() "
#endif

#define LOG(msg)        std::cout << Plunksna::getStr(Plunksna::Severity::eINFO) << LOG_SIG << msg << RESET << std::endl;
#define LOG_S(sev, msg) std::cout << Plunksna::getStr(sev)                       << LOG_SIG << msg << RESET << std::endl;
}

#endif //PLUNKSNALOG_H
