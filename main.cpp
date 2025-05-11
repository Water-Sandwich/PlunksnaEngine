#include <iostream>

#include "engine/PlunksnaEngine.h"
#include "engine/PlunksnaException.h"

int main()
{
    try {
        PlunksnaEngine engine("test", {640, 480}, 0);
        engine.init();
        engine.run();
        engine.clean();
    }
    catch (const PsnaExcp& e) {
        std::cout << "PSNAERR: " << e.what() << std::endl;
        return -2;
    }
    catch (const std::exception& e) {
        std::cout << "STDERR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
