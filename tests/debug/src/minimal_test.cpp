#include <iostream>
#include <unordered_map>

enum class LexerState {
  START,
  STRING_ESCAPE,
  STRING_SINGLE
};

int main() {
    std::cout << "Starting minimal test" << std::endl;
    
    std::unordered_map<LexerState, std::unordered_map<char, LexerState>> transitions_;
    
    std::cout << "Before setting transition" << std::endl;
    transitions_[LexerState::STRING_ESCAPE][static_cast<char>(-1)] = LexerState::STRING_SINGLE;
    std::cout << "After setting transition" << std::endl;
    
    std::cout << "Finished" << std::endl;
    return 0;
}