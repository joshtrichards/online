/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 */
#include "config.h"
#include "StringVector.hpp"
#include "Util.hpp"
#include "Log.hpp"
#include "Common.hpp"
#include "future"

extern "C" int forkit_main(int argc, char** argv);

char** convertStringVectorToCharArray(StringVector stringVector)
{
    char** charArray = new char*[stringVector.size()];

    for (size_t i = 0; i < stringVector.size(); ++i)
    {
        // Allocate memory for each C-style string and copy the content
        charArray[i] = new char[stringVector[i].size() + 1]; // +1 for null-terminator
        std::strcpy(charArray[i], stringVector[i].c_str());
    }

    return charArray;
}

extern "C" int createForkit(const std::string forKitPath, const StringVector args)
{
    // Create a promise to capture the result from the thread.
    std::promise<int> resultPromise;

    LOG_INF(forKitPath);
    char** argv = convertStringVectorToCharArray(args);

    LOG_INF("Run forkit main on a separate thread");

    int result = 0;
    std::thread worker = std::thread(
        [&]
        {
            Util::setThreadName("debug_forkit");
            result = forkit_main(args.size(), argv);
        });

    worker.join();

    // Clean up allocated memory for argv.
    for (size_t i = 0; i < args.size(); ++i)
    {
        free(argv[i]);
    }
    delete[] argv;
    LOG_INF("Forkit thread completed the task and result is " << result);

    // Return the result from the thread.
    return result;
}
