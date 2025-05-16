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

static constexpr std::string RED = "\033[49;31m";
static constexpr std::string YELLOW = "\033[49;33m";
static constexpr std::string WHITE = "\033[49;39m";
static constexpr std::string RESET = "\033[0m";

static constexpr std::string INFO = "[INFO]: ";
static constexpr std::string WARNING = "[WARNING]: ";
static constexpr std::string LETHAL = "[LETHAL]: ";

inline std::string getStr(Severity severity)
{
    switch (severity) {
    case Severity::eINFO:
        return WHITE + INFO;
    case Severity::eWARNING:
        return YELLOW + WARNING;
    case Severity::eLETHAL:
        return RED + LETHAL;
    default:
        return RESET + INFO;
    }
}

#define LOG(msg)        std::cout << Plunksna::getStr(Plunksna::Severity::eINFO) << msg << Plunksna::RESET << std::endl;
#define LOG_S(sev, msg) std::cout << Plunksna::getStr(sev)                       << msg << Plunksna::RESET << std::endl;
}

#endif //PLUNKSNALOG_H
