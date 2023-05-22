#include "application.hpp"

int main(int argc, char** argv) {
    // Run application and check status.
    std::error_code result;
    result = cpak::application::init();
    if (result.value() != EXIT_SUCCESS)
        std::cerr << result.message() << std::endl;

    result = cpak::application::run({ argv, argv + argc });
    if (result.value() != EXIT_SUCCESS)
        std::cerr << result.message() << std::endl;
        
    return result.value();
}