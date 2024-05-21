#include "id.h"

static std::random_device s_randomDevice;
static std::mt19937_64 s_mtEngine(s_randomDevice());
static std::uniform_int_distribution<uint64_t> s_uniformDistribution;

uint64_t ID::GenerateID()
{
    return s_uniformDistribution(s_mtEngine);
}
