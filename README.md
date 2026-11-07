# Better Pause Menu / QoL Hub

Better Pause Menu turns **Pause → Options → QoL Hub** into the central native RAC1 interface for quality-of-life tools.

It uses OpenRAC's native menu UI. It does not replace the game's original preview image and it does not add a duplicate gameplay bolt/health overlay.

## Integrated features

- **Status** — current planet, health, deaths, checkpoint state and latest progress.
- **Collectibles** — Gold Bolts, local remaining Gold Bolts, Infobots and skill-point triggers.
- **Missions** — lightweight current objective and recent progress events.
- **Fast Travel** — travel directly from the pause menu to unlocked planets.
- **Checkpoints** — temporary quick checkpoint create/restore plus optional warm auto-checkpoint.
- **Save Manager** — inspect real save slots and, on OpenRAC API 1.11+, save the active slot.
- **Gameplay Tools** — restart the current planet, return to the native game checkpoint (API 1.14+), and view session deaths.
- **Vendor Info** — current wallet and last acquired item.
- **Accessibility status** — reads Accessibility Pack configuration.
- **Cutscene Manager status** — reads the safe cutscene background module configuration.

## Consolidation

The Hub absorbs the in-game UI/functionality of these earlier standalone projects:

- Better HUD
- Collectible Tracker
- Mission Tracker
- Checkpoint Plus
- Death Improvements
- Improved Vendor

Those should stay disabled when the Hub is enabled.

**Fast Travel** and **Save Manager** are also integrated, but remain maintained as standalone packages for players who want only those features.

**Accessibility Pack** and **Cutscene Manager** remain separate background modules because their runtime behavior must continue outside the menu.

## Safety

- No game assets or disc files are included.
- Level-ending/progression cutscenes are not skipped by the Hub.
- Fast Travel only exposes planets that are already unlocked.
- Save writes target only the game's active save slot.
- The player's own installed game data is used at runtime.

Built for OpenRAC RAC1 PAL/NTSC-U. AI-assisted implementation. GPL-3.0-or-later.


## 0.8.5 always-visible Options information

The original left Options information box now shows live QoL status immediately whenever Pause → Options is open: planet, health, deaths, checkpoint, enabled mods and collectible progress. QoL Hub and Collectibles remain direct central menu entries. The original right-side game artwork/preview is not replaced or rewritten.

## Restart checkpoint

Open **Pause → Options → QoL Hub → Gameplay Tools → Restart checkpoint** and
confirm. The mod closes the pause menu through a fresh Start press, then asks
OpenRAC to respawn Ratchet through `hero_revive(nullptr, OPENRAC_REVIVE_RESPAWN)`.
This uses the game's ordinary death/checkpoint path, including its normal death
rules. It does not reload a save slot, restore a quick-checkpoint snapshot, or
travel back to the planet. Quick checkpoints and Restart current planet are
unchanged.

This action needs OpenRAC interface **1.14 or newer**. It is disabled with an
explanation on older ports; the rest of the Hub keeps its interface 1.9 minimum.
The build uses the pinned SDK 1.14.0 headers. A queued request is cancelled if the
level/load changes, a scene or loading transition begins, or the menu has not
closed within 120 ticks. The request runs only once after normal play resumes.

Validation before release: check the native confirmation and pause-menu exit on
PAL and NTSC-U, compare the respawn position/progress with an ordinary death,
verify cancellation and repeated use, and check coexistence with Quick checkpoints.
The automated tests use a fake API and do not prove in-game behavior.
