
#ifndef CYCLONITE_APP_COMMAND_LINE
#define CYCLONITE_APP_COMMAND_LINE

#include <unordered_set>
#include <string_view>

namespace cyclonite 
{
class CommadnLine
{
public:
    CommadnLine(int argc, const char* argv[]);

    [[nodiscard]] auto argumentCount() const -> uint32_t { return argc_; }
    
    [[nodiscard]] auto rawArguments() const -> char const** { return argv_; }
    
    [[nodiscard]] auto arguments() const -> std::unordered_set<std::string_view> const& { return arguments_; }

private:
    uint32_t argc_;
    char const** argv_;
    std::unordered_set<std::string_view> arguments_;
};
}

#endif // CYCLONITE_APP_COMMAND_LINE
