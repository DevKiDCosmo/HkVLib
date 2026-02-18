#pragma once

#include "../include.h"

class Log
{
public:
    static void sys_info(const char *Tag, String msg);
    static void sys_infoflag(const char *Tag, String msg, bool Flag);

    static void sys_warning(const char *Tag, String msg);
    static void sys_error(const char *Tag, String msg);
};