 #include "SDL.h" // To substitute main with SDL_main

#include "Core/Application.hpp"

int main(int argc, char** argv) {
    Application app;

    app.run();

    return 0;
}