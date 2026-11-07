#pragma once
#include <cstdint>
#include "openrac/openrac_mod.h"

namespace qol {

inline bool has_native_checkpoint_restart(const openrac_api& api) {
    return api.api_major == 1 && api.api_minor >= 14
        && api.size >= offsetof(openrac_api, hero_revive) + sizeof(api.hero_revive)
        && api.hero_revive != nullptr;
}

inline bool restart_native_checkpoint(const openrac_api& api, openrac_mod* mod) {
    return has_native_checkpoint_restart(api)
        && api.hero_revive(mod, nullptr, OPENRAC_REVIVE_RESPAWN) == OPENRAC_HOOK_OK;
}

// A one-shot request, tied to the current level load, not a saved snapshot.
class CheckpointRestart {
public:
    static bool allowed(const openrac_play_status& status) {
        return status.level >= 0 && (status.phase == OPENRAC_PLAY_MENU
                                    || status.phase == OPENRAC_PLAY_PLAYING);
    }

    void request(const openrac_play_status& status) {
        cancel();
        if (!allowed(status)) return;
        level_ = status.level;
        loads_ = status.loads;
        pending_ = true;
    }

    bool pending() const { return pending_; }
    void cancel() { pending_ = false; pad_step_ = 0; ticks_ = 0; }

    bool tick(const openrac_play_status& status) {
        if (!pending_) return false;
        if (!same_load(status) || !allowed(status) || ++ticks_ > 120) {
            cancel();
            return false;
        }
        if (status.phase != OPENRAC_PLAY_PLAYING) return false;
        cancel();
        return true;
    }

    // Close the ordinary pause menu using one fresh Start press. Do not alter
    // navigation buttons/axes or inject anything once play has resumed.
    void pad(const openrac_play_status& status, std::uint8_t* report, int size) {
        if (!pending_ || !same_load(status) || status.phase != OPENRAC_PLAY_MENU
            || report == nullptr || size < 2 || pad_step_ >= 3) return;
        if (pad_step_++ == 1) {
            report[0] &= static_cast<std::uint8_t>(~OPENRAC_PAD_START);
        } else {
            report[0] |= OPENRAC_PAD_START;
        }
    }

private:
    bool same_load(const openrac_play_status& status) const {
        return status.level == level_ && status.loads == loads_;
    }
    bool pending_ = false;
    int level_ = -1;
    std::uint32_t loads_ = 0;
    unsigned ticks_ = 0;
    unsigned pad_step_ = 0;
};

}  // namespace qol
