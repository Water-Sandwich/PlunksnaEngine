//
// Created by d on 5/11/25.
//

#include "Exception.h"

namespace Plunksna {

Exception::Exception(const std::string& what) :
    m_what(std::string(Plunksna::getStr(Plunksna::Severity::eLETHAL)) + what) {}

const char* Exception::what() const noexcept
{
    return m_what.c_str();
}

} //Plunksna
