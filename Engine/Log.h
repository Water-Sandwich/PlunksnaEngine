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
    eVERBOSE,
    eWARNING,
    eLETHAL
};

//colors
#define RED "\033[49;31m"
#define YELLOW "\033[49;33m"
#define WHITE "\033[49;39m"
#define RESET "\033[0m"

//prefixes
#define PINFO "[INFO]: "
#define PWARNING "[WARNING]: "
#define PLETHAL "[LETHAL]: "
#define PVERBOSE "[VERBOSE]: "

constexpr const char* getStr(Severity severity)
{
    switch (severity) {
    case Severity::eINFO:
        return WHITE PINFO;
    case Severity::eWARNING:
        return YELLOW PWARNING;
    case Severity::eLETHAL:
        return RED PLETHAL;
    default:
        return RESET PINFO;
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
#define LOG_C(msg)      std::cout << Plunksna::getStr(Plunksna::Severity::eINFO) << msg << RESET << std::endl;
}

#endif //PLUNKSNALOG_H
