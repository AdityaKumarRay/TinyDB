#include <iostream>
#include <string>

// Phase 1: Minimal Skeleton - A basic REPL
int main(int argc, char* argv[]) {
    std::string input_buffer;

    while (true) {
        std::cout << "db > ";
        
        if (!std::getline(std::cin, input_buffer)) {
            break; // EOF
        }

        if (input_buffer.empty()) {
            continue;
        }

        if (input_buffer == ".exit") {
            break;
        } else {
            std::cout << "Unrecognized command '" << input_buffer << "'.\n";
        }
    }

    return 0;
}
