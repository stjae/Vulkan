// Generation of random IDs for objects
// オブジェクト用ランダムIDの生成

#ifndef ID_H
#define ID_H

#include <random>

class ID
{
public:
    static uint64_t GenerateID();
};

#endif