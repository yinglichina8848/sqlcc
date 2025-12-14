#include <iostream>
#include <unordered_map>

enum class LexerState {
  COMMENT_BLOCK_STAR,
  COMMENT_BLOCK
};

using TransitionMap =
    std::unordered_map<LexerState, std::unordered_map<char, LexerState>>;

int main() {
    std::cout << "Starting debug loop test" << std::endl;
    
    TransitionMap transitions_;
    
    std::cout << "Starting loop" << std::endl;
    int count = 0;
    for (char c = 0; c < 128; c++) {
        if (c != '/' && c != '*') {
            transitions_[LexerState::COMMENT_BLOCK_STAR][c] = LexerState::COMMENT_BLOCK;
            count++;
            if (count % 10 == 0) {
                std::cout << "Processed " << count << " characters" << std::endl;
            }
        }
    }
    
    std::cout << "Loop completed, processed " << count << " characters" << std::endl;
    std::cout << "Test completed successfully" << std::endl;
    return 0;
}