#include "./host_memory_allocator.h"

#include "../memory_management/memory_management.h"

namespace UnitTest
{
    bool runRtosHostMemoryAllocatorTest()
    {
        return runRtosMemoryManagementSuite();
    }
} // namespace UnitTest
