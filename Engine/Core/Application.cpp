#include "Core/Application.hpp"

#include <algorithm>

#include "SDL.h"

#include "Core/ClipRegistry.hpp"
#include "Core/Concurrency/Job.hpp"
#include "Core/Concurrency/JobQueue.hpp"
#include "Core/FontRegistry.hpp"
#include "Core/SpriteRegistry.hpp"
#include "Core/TextureRegistry.hpp"
#include "GFX/Animation/AnimationSystem.hpp"
#include "GFX/Render.hpp"
#include "GFX/Window.hpp"
#include "Input/Input.hpp"
#include "Memory/DoubleEndedLinearAllocator.hpp"
#include "Memory/LinearAllocator.hpp"
#include "Memory/ScopeStack.hpp"
#include "Memory/SmallObjectPool.hpp"
#include "String.hpp"
#include "Util/Registry.hpp"

static const int64_t Second = SDL_GetPerformanceFrequency();
static const int64_t UpdateFrequency = 60;
static const int64_t UpdateInterval = Second / UpdateFrequency;
static const int64_t MaxTimeDiff = Second / 4;

static const size_t MaxUpdateCount = 5;

static const size_t AppHeapSize = 64 * 1024 * 1024;
static const size_t ScratchBufferSize = 1024;

int64_t getSystemTicks() {
    return SDL_GetPerformanceCounter();
}

Application::Application() NOEXCEPT :
    _updateTps {0},
    _renderFps {0},
    _lastTime {0},
    _newTime {0},
    _lastSecond {0},
    _done {false}
{
    const auto initialized =
        SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO) == 0;

    if (!initialized) {
        SDL_Log("Could not initialize: %s", SDL_GetError());
        exit(1);
    }
}

Application::~Application() {
    SDL_Quit();
}

static uint8_t appHeap[AppHeapSize];

void Application::run() {
    AppAlloc appAlloc(std::begin(appHeap), std::end(appHeap));

    SmallObjectPool::DefaultInstance smallObjectPool(appAlloc);

    JobQueue::DefaultInstance jobQueue(appAlloc);

    TextureRegistry::DefaultInstance textureRegistry(appAlloc);
    SpriteRegistry::DefaultInstance spriteRegistry(appAlloc);
    FontRegistry::DefaultInstance fontRegistry(appAlloc);
    ClipRegistry::DefaultInstance ClipRegistry(appAlloc);

    AnimationSystem::DefaultInstance animationSystem(appAlloc);

    //TODO: load stuff

    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(0, &mode);

    ScopeStack<AppAlloc> appScope(appAlloc);

    Window* window = appScope.create<Window>(mode.w, mode.h);
    mainLoop(window, appAlloc);
}

void Application::mainLoop(Window* window, AppAlloc& alloc) {
    ScopeStack<AppAlloc> mainScope(alloc);

    Input input(mainScope);
    input.applicationExitRequested.subscribe(make_delegate([this]() {
        _done = true;
    }));

    Render render(mainScope);

    uint8_t scratchBuffer[ScratchBufferSize];
    LinearAllocator scratch(std::begin(scratchBuffer), std::end(scratchBuffer));

    int64_t accumulatedTime = 0;
    while (!_done) {
        ScopeStack<LinearAllocator> frameScope(scratch);

        input.processEvents(window);

        _lastTime = _newTime;
        _newTime = getSystemTicks();

        const int64_t diff = std::min(_newTime - _lastTime, MaxTimeDiff);

        if (_newTime - _lastSecond >= Second) {
            _renderFps = 0;
            _updateTps = 0;
            _lastSecond += Second;
        }

        accumulatedTime += diff;
        size_t updateCount = 0;
        while (accumulatedTime >= UpdateInterval && updateCount < MaxUpdateCount) {
            accumulatedTime -= UpdateInterval;

            //TODO: update game and interpolate state

            ++_updateTps;
            ++updateCount;
        }

        render.render(frameScope, window);

        window->swapBuffers();

        ++_renderFps;
    }
}
