//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNAEXCEPTION_H
#define PLUNKSNAEXCEPTION_H


#include <exception>
#include <string>
#include "PlunksnaLog.h"

class PlunksnaException : public std::exception
{
private:
    std::string m_what;

public:
    PlunksnaException(const std::string& what);
    const char* what() const noexcept override;
};

using PsnaExcp = PlunksnaException;

#define THROW_IF_NULL(sptr, msg)   if(!sptr.get()){throw PsnaExcp(msg);}
#define THROW_IF_NULL_R(ptr, msg)  if(!ptr){throw PsnaExcp(msg);}


#endif //PLUNKSNAEXCEPTION_H
