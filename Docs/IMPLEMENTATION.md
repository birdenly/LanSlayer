## How It Works

### 1. Title Storage Loop
On startup and when entering Multiplayer, the game calls `idSessionLocal::UpdateTitleStorage` to download configuration data (`tss.bin`) from Bethesda servers. When offline, this request never completes and leaves the game stuck on `NOTICE: Loading title storage.`

The executable is patched to take the built-in disabled routine in `UpdateTitleStorage` and force `IsTitleStorageReady()` to return `true`, skipping the loading screen on the first frame.

### 2. Dedicated Server Gatekeeper & Endpoint Checks
Selecting **Private Match** queries Bethesda's gatekeeper server (`198.20.198.48/gatekeeper`) and pings dedicated server endpoints. When unreachable, the game blocks access with `Unable to reach dedicated server endpoints.`

Both `IsDedicatedServerGatePassed` and the endpoint ping check are patched to return `true` immediately, unlocking the Private Match menu without any network dependency.

### 3. Private Match Local Hosting
Starting a Private Match originally required a cloud dedicated server (`State_Find_Dedicated_Instance`). If none was available, it failed with `Unable to find a dedicated server.` It also prevented starting a match with fewer than 2 players.

The lobby and session start routines (`CanStartPrivateMatch`, `StartMatch`, and minimum players) were patched to bypass dedicated server allocation and host locally as a listen server over P2P.

### 4. LAN Join & P2P Connection Fix
When a second player joins a Private Match lobby over LAN, the `OnP2PSessionRequest` callback called `SteamFriends()->GetFriendPersonaName()` before accepting the session. On LAN where players are not on an online Steam friends list, this caused an access violation crash (`0xC0000005`) on the host.

The redundant persona name lookup was removed so `AcceptP2PSessionWithUser` executes immediately, allowing players to join lobbies seamlessly.

### 5. Local XP, Leveling & Progression
By default, Private Matches disabled all XP and stat gains (`metrics_allowStatsAndXPInPrivateMatch = 0`), hid HUD XP popups (`hud_showXPGainsInPrivateMatch = 0`), cleared the post-match summary (`idGameLocal::UpdateMatchResults`), and kept the Title Storage variable `giveXP` locked to `0`.

These checks were patched in the executable so kills, medals, challenges, post-match XP summaries, in-game taunts (`allowTaunts`), and level-ups work in Private Matches and save directly to your local profile (`profile.bin`). You can customize `+metrics_xpMultiplier`

### 6. SnapMap LAN Co-op
Retail SnapMap was designed around Bethesda Hydra cloud servers for account authentication, hub menus, and map validation. Without internet connection, entering SnapMap would hang on `NOTICE: Signing into DOOM Services` (Dialog `0x97`), lock menu tabs, and prevent starting multiplayer lobbies.

The executable is patched to bypass online account and privilege checks, unlock all SnapMap tabs, disable HMAC signature validation (`CYBERDEMON`) to allow playing maps from any source, and redirect matchmaking into a local P2P listen server lobby. Network timeouts (`CheckHeartbeats`, `MapFinishedLoading`, `net_peerTimeoutDuringLoadMS`) and packet congestion drops were also patched to ensure players stay connected and load together smoothly over LAN.
