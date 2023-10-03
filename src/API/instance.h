#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"

class Instance
{
public:
    Instance();
    ~Instance();

private:
    vk::Instance m_instance;
};

#endif