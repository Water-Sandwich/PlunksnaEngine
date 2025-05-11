//
// Created by d on 5/11/25.
//

#include "PlunksnaException.h"

PlunksnaException::PlunksnaException(const std::string& what) :
    m_what(Logs::getStr(Logs::Severity::eLETHAL) + what) {}

const char* PlunksnaException::what() const noexcept
{
    return m_what.c_str();
}
