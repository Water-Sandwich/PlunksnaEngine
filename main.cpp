#include <iostream>

#include "engine/Engine.h"
#include "engine/Exception.h"

int main()
{
    try {
        Plunksna::Engine engine("test", {640, 480}, 0);
        engine.init();
        engine.run();
        engine.clean();
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