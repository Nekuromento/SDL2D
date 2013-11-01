#include "Job.hpp"

#include <cassert>

Job::Job() :
    function {nullptr},
    payload {nullptr}
{
}

Job::Job(ConstJobFunction function, void* const payload /*= nullptr*/) :
    function {function},
    payload {payload}
{
}

void Job::run() {
    assert(("Trying to execute empty job", function));
    function(payload);
}
