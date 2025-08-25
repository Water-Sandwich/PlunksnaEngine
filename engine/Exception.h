//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNAEXCEPTION_H
#define PLUNKSNAEXCEPTION_H


#include <exception>
#include <string>
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

#define THROW_IF_NULL(sptr, msg)   if(!sptr.get()){throw Plunksna::Exception(msg);}
#define THROW_IF_NULL_R(ptr, msg)  if(!ptr){throw Plunksna::Exception(msg);}
#define THROW_IF_EQ(expr, msg)     if(expr){throw Plunksna::Exception(msg);}
#define THROW_IF_NEQ(expr, msg)    if(!expr){throw Plunksna::Exception(msg);}


#endif //PLUNKSNAEXCEPTION_H
