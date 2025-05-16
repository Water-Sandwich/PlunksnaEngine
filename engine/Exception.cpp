//
// Created by d on 5/11/25.
//

#include "Exception.h"

using namespace Plunksna;

Exception::Exception(const std::string& what) :
    m_what(Plunksna::getStr(Plunksna::Severity::eLETHAL) + what) {}

const char* Exception::what() const noexcept
{
    return m_what.c_str();
}
