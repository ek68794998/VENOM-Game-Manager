#VENOM Game Manager (VGM)
As part of an ongoing effort to keep the classic 2002 game, [Command & Conquer: Renegade](https://en.wikipedia.org/wiki/Command_%26_Conquer:_Renegade) fresh, exciting, and bug-free, VGM was created as a server-side utility to facilitate such goals.

##Features

- Multiple game modes, including all-out war, sniper-only, and infantry-only
- Crate boxes containing random effects (weapons, etc.)
- A commander feature, allowing a team to have a commander to organize tasks
- A Tiberium Crystal feature, allowing teams to return the crystal to their base for money
- A veteran system, allowing players to rank up (and unlock new abilities) as they get kills on players, vehicles, and buildings
- Medals, like achievements, for players doing things such as spree kills or destroying X buildings
- Ability to enter commands into the console to do things such as:
 - Muting, moving, or killing players
 - Allowing an in-game player to invisibly spectate others
- Allows players to lock vehicles to prevent team theft
- Allows moderators to view in-game events such as damage values, flags for headshots, and so on
 - Some of this information is available to players in-game as well
 - Also allows users and moderators to view information via IRC, such as building status (health), without being in-game
- Vehicle shells, including shells for Harvesters and helicopters (experimental), to allow vehicles to be repaired
- Adds gun towers for GDI to mirror Nod's turrets
- Allows permission to reserve slots (for moderators) which can bypass the player limit
- Custom radio commands and ability to do poses by pressing numpad keys
- Players now drop weapon packs so that others can pick up more weapons
- Lots of other small additions!

##Anti-cheat and moderator tools

- Some console commands and moderator tools mentioned above help serve this purpose
- Blocks (and detects) bighead hack users
- Blocks (and detects) damage hack users
- Blocks (and detects) rapid-fire hack users
- Users can be banned by serial, IP range, or hostname
- Blocks users from bypassing DNS hacks
- Blocks users from hacking serial codes
- Blocks users from crashing server using a connection exploit
- The base of a primitive aimbot detector is found in the source, though this was never completed

##Bug patches

- Credits are now distributed to the team evenly during a Harvester unload, instead of in one lump sum
- Obelisk will no longer fail at targeting objects due to them going out of sight for a split second
- Advanced Guard Tower will no longer continue to shoot at objects which have disappeared from its line of sight
- Fix for getting too many points from vehicle-vs-soldier battles
- Fix for using purchase terminals through walls
- Fix for pistol not being loaded
- Fix for "blue hell" (off the map) glitch
- Some minor netcode fixes
- Some minor console command fixes
- Other minor fixes

##Notes
Due to the team-based nature of the development of VGM, some components (such as the Obelisk and Guard Tower fixes) have been omitted from the source code.

This project was compiled using the `scripts-3.4.4` open-source base (formerly available from [SourceForge](http://sourceforge.net/projects/rentools/files/) prior to `scripts-4.0`).