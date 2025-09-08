
#include <cyclonite.h>

namespace examples 
{
class BasicExample
{
public:
    BasicExample() = default;

    auto init(cyclonite::CommadnLine const& commandLine) -> BasicExample&; 
    
    auto run() -> BasicExample&; 

    void done();

private:
    cyclonite::Root root_;
};
}
