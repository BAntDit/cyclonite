//
// Created by anton on 5/24/26.
//

#ifndef CYCLONITE_GFX_FRAMERE_CORDING_CONTEXT_H
#define CYCLONITE_GFX_FRAMERE_CORDING_CONTEXT_H

#include <future>
#include <vector>

namespace cyclonite::gfx {
class FrameRecordingContext
{
public:
    explicit FrameRecordingContext(uint64_t frameIndex)
      : currentFrameIndex_{ frameIndex }
    {
    }

    [[nodiscard]] auto recordingTasks() -> std::vector<std::future<void>>& { return futures_; }

    [[nodiscard]] auto recordingTasks() const -> std::vector<std::future<void>> const& { return futures_; }

    [[nodiscard]] auto currentFrameIndex() const -> uint64_t { return currentFrameIndex_; }

    void addTask(std::future<void>&& recordingTask) { futures_.emplace_back(std::move(recordingTask)); }

private:
    uint64_t currentFrameIndex_;
    std::vector<std::future<void>> futures_;
};
}

#endif // CYCLONITE_GFX_FRAMERE_CORDING_CONTEXT_H