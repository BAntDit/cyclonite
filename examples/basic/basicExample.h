
#include <cyclonite.h>

namespace examples {
class BasicExample : public cyclonite::EventReceivable
{
public:
    BasicExample();

    auto init(cyclonite::CommandLine const& commandLine) -> BasicExample&;

    auto run() -> BasicExample&;

    void done();

    void onQuit() { shutdown_ = true; }

    void onKeyDown(SDL_Keycode keyCode, uint16_t mod);

private:
    cyclonite::Root<> root_;
    cyclonite::core::ResourceSharedRef deviceRef_;
    cyclonite::core::ResourceSharedRef renderPassRef_;
    cyclonite::core::ResourceSharedRef windowRef_;
    std::unique_ptr<cyclonite::gfx::QueueSubmissionManager> submissionManager_;
    bool shutdown_;
};
}
