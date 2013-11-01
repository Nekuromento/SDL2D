#include "JobQueue.hpp"

#include <algorithm>

#include "SDL_thread.h"

int JobQueue::Worker::threadRun(void* data) {
    static_cast<JobQueue::Worker*>(data)->run();
    return 0;
}

JobQueue::Worker::Worker(State& state) :
    state(state), //XXX: gcc bug prevents from using brace initialization syntax
    done {false},
    thread {SDL_CreateThread(&threadRun, "JobQueue::Worker", this)}
{
}

void JobQueue::Worker::run() {
    Job job;

    while (!done) {
        if (state.begin >= state.end)
            continue;

        // fetch new job
        job = state.queue[state.begin & (MaxJobCount - 1)];

        ++state.begin;

        job.run();
    }
}

JobQueue::~JobQueue() {
    _worker.done = true;

    SDL_WaitThread(_worker.thread, nullptr);
}

void JobQueue::add(const Job& job) {
    assert(("Job queue is too busy", _state.end - _state.begin < MaxJobCount));

    _state.queue[_state.end & (MaxJobCount - 1)] = job;
    ++_state.end;
}

JobQueue* JobQueue::DefaultInstance::defaultInstance;

JobQueue::DefaultInstance::~DefaultInstance() {
    assert(("Default instance already destroyed", defaultInstance));
    defaultInstance->~JobQueue();
    defaultInstance = nullptr;
}

JobQueue& JobQueue::getDefault() {
    return *DefaultInstance::defaultInstance;
}
