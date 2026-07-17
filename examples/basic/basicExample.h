
#include <cyclonite.h>
#include <systems/renderSystem.h>

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

    void onMouseButtonDown(uint8_t button, uint8_t clicks, float x, float y);

    void onMouseButtonUp(uint8_t button, float x, float y);

    void onMouseMotion(float x, float y); // TODO:: add arguments if button pressed and button index

    void onMouseWheel(float y);

private:
    cyclonite::Root<> root_;
    cyclonite::core::ResourceSharedRef deviceRef_;
    cyclonite::core::ResourceSharedRef windowRef_;
    cyclonite::core::ResourceSharedRef materialRef_;
    cyclonite::systems::Renderer renderer_;
    uint32_t defaultResourceGroup_;

    cyclonite::real scroll_;
    cyclonite::real fade_;
    cyclonite::real distance_;
    cyclonite::real distanceMin_;
    cyclonite::real distanceMax_;
    cyclonite::real azimuth_;
    cyclonite::real polar_;
    cyclonite::vec2 rotate_;
    cyclonite::vec2 rotationStart_;
    cyclonite::vec3 target_;

    std::chrono::time_point<std::chrono::high_resolution_clock> time_;

    bool isInRotation_;
    bool shutdown_;
};
}
