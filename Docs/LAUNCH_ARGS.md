# Launch args
### Rendering API
`+r_renderAPI 0` OpenGL

`+r_renderAPI 1` Vulkan

`+com_skipIntroVideo 1` Skips intro videos

`+net_dedicatedServerGate_Enable 0` Disables the online check that queries Bethesda's dedicated server gatekeeper before unlocking Multiplayer/Private Match menus.

`+net_peerTimeoutDisabled 1` Completely disables CheckHeartBeats and IsLosingConnectionToHost timeout kicks, preventing players from dropping when disk I/O pauses heartbeats.

`+net_peerTimeoutDuringLoadMS 120000` Time (in ms; 120000 = 2 minutes) the host waits for a peer during map load before giving up and entering the match without them.

`+net_peerTimeoutInSeconds 120` Time (in seconds) without a response before a connected peer is disconnected in-game.

`+net_peerTimeoutInSeconds_Lobby 120` Time (in seconds) without a response before a peer is disconnected while in the lobby.

`+net_connectTimeoutInSeconds 120` Maximum time (in seconds) allowed to establish an initial connection to a lobby/host.

`+net_waitOnPeerToLoadTimer_Seconds 120` After a client finishes joining/loading, how long (in seconds) to wait for remaining clients before forcing the match to start.

`+net_waitOnPeersToLoadMinimumCount 2` Minimum number of clients required before the host is allowed to force everyone into the match if other peers are still loading.

`+net_forceStartMatchTimeLimit 120` Maximum time (in seconds) before allowing a forced match start.

`+com_logFile 2` Logs to `%USERPROFILE%\Saved Games\id Software\DOOM\base\qconsole.log`

0: Disabled

1: Writes logs to disk periodically or when the game closes.

2: Writes each line immediately, ideal for diagnosing freezes and crashes.

`+menu_allowPrivateMatch 1` Forces the Private Match option to remain enabled in the menu.

`+menu_startMatchAlwaysAllowed 1` Allows the host to start a match from the lobby regardless of normal readiness/player restrictions.

`+gc_minPlayersInPrivateMatch 1` Lowers the minimum required players to 1 so Private Matches don't immediately end or refuse to start when playing solo or with fewer than 2 players.

`+metrics_allowStatsAndXPInPrivateMatch 1` Enables earning stats, medals, and XP in Private Matches.

`+hud_showXPGainsInPrivateMatch 1` Displays +XP popups on the HUD during Private Matches.

`+metrics_xpMultiplier 0` Scalar multiplier applied to earned XP

### SnapMap
`+com_gameMode 1`

`+com_gameType 1`

### DevMode
`+devMode_enable 1`

`+devMode_skipCorruptedSaves 1`
