#pragma once

class String;

namespace UnitTest
{
    namespace FsInternal
    {
        struct FsContext
        {
            bool mountedByTest = false;
            const char *spiffsLabel = nullptr;
            const char *mount = nullptr;
        };

        bool hasDirectory(const char *path);
        bool ensureFsContext(FsContext &ctx);
        void releaseFsContext(const FsContext &ctx);

        bool existsPath(const String &path);
        bool cleanupPathIfExists(const String &path);
    } // namespace FsInternal
} // namespace UnitTest
