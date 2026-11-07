#include <stdexcept>
#include "restart_checkpoint.hpp"

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (0)

static int calls = 0;
static int result = OPENRAC_HOOK_OK;
static int revive(openrac_mod*, const float* position, std::uint32_t flags) {
    CHECK(position == nullptr);
    CHECK(flags == OPENRAC_REVIVE_RESPAWN);
    ++calls;
    return result;
}

int main() {
    openrac_play_status menu{};
    menu.size = sizeof menu;
    menu.level = 2;
    menu.loads = 5;
    menu.phase = OPENRAC_PLAY_MENU;
    auto play = menu;
    play.phase = OPENRAC_PLAY_PLAYING;
    qol::CheckpointRestart restart;

    // A confirmed request leaves the menu with a fresh Start edge, then fires once.
    restart.request(menu);
    CHECK(!restart.tick(menu));
    std::uint8_t report[] = {0x00, 0xA5, 127, 128};
    restart.pad(menu, report, sizeof report);
    CHECK((report[0] & OPENRAC_PAD_START) != 0);
    restart.pad(menu, report, sizeof report);
    CHECK((report[0] & OPENRAC_PAD_START) == 0);
    restart.pad(menu, report, sizeof report);
    CHECK((report[0] & OPENRAC_PAD_START) != 0);
    CHECK(report[1] == 0xA5 && report[2] == 127 && report[3] == 128);
    CHECK(restart.tick(play));
    CHECK(!restart.tick(play));
    CHECK(!restart.pending());

    // Invalid/truncated input does not advance the close-menu sequence.
    restart.request(menu);
    restart.pad(menu, nullptr, 0);
    restart.pad(menu, report, 1);
    report[0] = 0;
    restart.pad(menu, report, sizeof report);
    CHECK((report[0] & OPENRAC_PAD_START) != 0);

    // Never carry a request into a new load, another planet, a cutscene or a menu timeout.
    for (int scenario = 0; scenario < 4; ++scenario) {
        restart.request(menu);
        auto changed = play;
        if (scenario == 0) ++changed.loads;
        if (scenario == 1) ++changed.level;
        if (scenario == 2) changed.phase = OPENRAC_PLAY_SCENE;
        if (scenario == 3) changed.phase = OPENRAC_PLAY_LOADING;
        CHECK(!restart.tick(changed));
        CHECK(!restart.pending());
        CHECK(!restart.tick(play));
    }
    restart.request(menu);
    for (int i = 0; i <= 120; ++i) CHECK(!restart.tick(menu));
    CHECK(!restart.tick(play));
    auto title = menu;
    title.level = -1;
    title.phase = OPENRAC_PLAY_FRONT_END;
    restart.request(title);
    CHECK(!restart.pending());
    restart.request(play);
    CHECK(restart.tick(play));

    // Optional 1.14 API: old or shortened tables never reach the new function.
    openrac_api api{};
    api.size = sizeof api;
    api.api_major = 1;
    api.api_minor = 9;
    api.hero_revive = revive;
    CHECK(!qol::restart_native_checkpoint(api, nullptr));
    api.api_minor = 14;
    api.size = offsetof(openrac_api, hero_revive);
    CHECK(!qol::restart_native_checkpoint(api, nullptr));
    CHECK(calls == 0);
    api.size = sizeof api;
    CHECK(qol::restart_native_checkpoint(api, nullptr));
    CHECK(calls == 1);
    result = OPENRAC_HOOK_UNKNOWN;
    CHECK(!qol::restart_native_checkpoint(api, nullptr));
    api.hero_revive = nullptr;
    CHECK(!qol::has_native_checkpoint_restart(api));
}
