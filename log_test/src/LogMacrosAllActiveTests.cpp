
// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "string.h"
#include "log/Log.hpp"
#define FASTDDS_ENFORCE_LOG_INFO
#ifdef HAVE_LOG_NO_INFO
#undef HAVE_LOG_NO_INFO
#endif // HAVE_LOG_NO_INFO
#define HAVE_LOG_NO_INFO 0

#ifdef HAVE_LOG_NO_WARNING
#undef HAVE_LOG_NO_WARNING
#endif // HAVE_LOG_NO_WARNING
#define HAVE_LOG_NO_WARNING 0

#ifdef HAVE_LOG_NO_ERROR
#undef HAVE_LOG_NO_ERROR
#endif // HAVE_LOG_NO_ERROR
#define HAVE_LOG_NO_ERROR 0

#include <log/Log.hpp>
/* Check all log levels are actived and consumed
 * This test's name can be misunderstood. All active refers to all log level activated, that is why
 * all define clauses are set to 0 (negative macros)
 */

using namespace  eprosima::fastdds::dds;

int main(int argc, char** argv)
{

        Log::SetVerbosity(Log::Info);
        logError(SampleCategory, "Sample error message");
        logWarning(SampleCategory, "Sample warning message");
        logInfo(SampleCategory, "Sample info message");
        getchar();

    return 0;
}
