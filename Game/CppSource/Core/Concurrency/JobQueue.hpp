#ifndef JobQueue_h__
#define JobQueue_h__

#include <cassert>
#include <cstdint>
#include <type_traits>

#include "Job.hpp"
#include "Util/noncopyable.hpp"

struct SDL_Thread;

class JobQueue : public util::Noncopyable {
    static const size_t MaxJobCount = 32;
    static_assert((MaxJobCount & (MaxJobCount - 1)) == 0, "MaxJobCount must be power of two");

    static const size_t JobStorageSize = MaxJobCount * sizeof(Job);
    static const size_t JobAlignment = std::alignment_of<Job>::value;

    struct State {
        Job* queue;
        size_t begin;
        size_t end;

        template <typename Allocator>
        State(Allocator& alloc) :
            queue {static_cast<Job*>(alloc.allocate(JobStorageSize, JobAlignment, 0))},
            begin {0},
            end {0}
        {}
    };

    // make worker an object to enable multiple workers, if needed
    struct Worker {
        State& state;
        bool done;
        SDL_Thread* thread;

        static int threadRun(void* data);

        Worker(State& state);

        void run();
    };

    State _state;
    Worker _worker;

public:
    struct DefaultInstance {
        static JobQueue* defaultInstance;

        template <typename Allocator>
        DefaultInstance(Allocator& alloc) {
            assert(("Trying to initialize default instance twice", !defaultInstance));
            void* const memory =
                alloc.allocate(sizeof(JobQueue), std::alignment_of<JobQueue>::value, 0);
            defaultInstance = new (memory) JobQueue(alloc);
        }
        ~DefaultInstance();
    };

    static JobQueue& getDefault();

    template <typename Allocator>
    JobQueue(Allocator& alloc) :
        _state {alloc},
        _worker {_state}
    {}

    ~JobQueue();

    void add(const Job& job);
};

#endif // JobQueue_h__