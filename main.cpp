#include <iostream>

#include "engine/Engine.h"
#include "engine/Exception.h"

int main()
{
    try {
        SDL_Init(SDL_INIT_VIDEO);
        Plunksna::Engine engine("test", {640, 480}, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        engine.run();
    }
    catch (const Plunksna::Exception& e) {
        std::cout << "PSNAERR: " << e.what() << std::endl;
        return -2;
    }
    catch (const std::exception& e) {
        std::cout << "STDERR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}