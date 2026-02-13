//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNAEXCEPTION_H
#define PLUNKSNAEXCEPTION_H


#include <exception>
#include <string>
#include <sstream>
#include "Log.h"

namespace Plunksna {
class Exception : public std::exception
{
private:
    std::string m_what;

public:
    Exception(const std::string& what);
    const char* what() const noexcept override;
};

}

#define THROW(msg)                  throw Plunksna::Exception(msg);
#define THROW_SS(msg)               std::stringstream ss; ss << msg; throw Plunksna::Exception(ss.str());

#define ASSERT(exp, msg)            if(!(exp)) {THROW(msg);}
#define CHECK_R(exp, msg)          if(!(exp)) {LOG_S(eWARNING, msg); return;}
#define ASSERT_SS(exp, msg)         if(!(exp)) {THROW_SS(msg);}
#define ASSERT_V(res1, msg)         if((res1) != VK_SUCCESS) {THROW(msg);}


#endif //PLUNKSNAEXCEPTION_H
