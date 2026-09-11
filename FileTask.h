#pragma once
#include "BusinessTask.h"
class FileTask :
    public BusinessTask
{
public:
    FileTask(void* data, int size, int fd);

    void execute();

    static bool savePPM(const std::string& path,
        const std::vector<uint8_t>& data,
        int w, int h);
};

