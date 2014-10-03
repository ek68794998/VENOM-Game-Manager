#pragma once

#include "vgm_engine.h"
#include "vgm_game.h"
#include "vgm_net.h"
#include "vgm_obj.h"
#include "vgm_player.h"
#include "vgm_weapon.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

#define InitHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define XWISInitHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define EmergencyShutdownHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ShutdownHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ThinkHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define LevelLoadHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define LevelEndHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define TeamScoreSwitchFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ChangeTeamHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ResetGameHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ChatMsgHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define RadioHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ReserveAllSlotsHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define XWISPlayerLimitBypassHookAddr1 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define XWISPlayerLimitBypassHookAddr2 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ClientConnectionHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PlayerJoinHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ClientDisconnectHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PlayerLeaveHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PlayerTeamHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define SerialRetrievedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define VehicleTransitionHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ExitDeadVehicleHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define OnPurchaseHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define OnPurchaseCharacterHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PowerupCollectedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define WantsPowerupsHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define WeaponDefSearchHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PlayerKillMsgHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define DamageHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define SoldierRDHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define VehicleRDHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BuildingRDHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define FatalDamageHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define JumpHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define FallHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define VehicleFlipHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ObjectCreationHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define VehicleCreateHookAddr1 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define VehicleCreateHookAddr2 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define DistributeFundsHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define HarvesterDockedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define OldHarvesterDistributionHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4InitHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4LimitHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4ShouldExplodeHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4DefuseHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4DetonationHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
//#define C4DetonationHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4PlantedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define C4ExportAttachedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define EnemySeenHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BeaconStateHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BeaconWarningHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BeaconInitiatedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BeaconDisarmedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BeaconRequestedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define BeaconOwnerInterruptedHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PedestalTimerHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PedestalHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define SelectWeaponHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define UnlimitedAmmoHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#ifdef _DEV_
#define obbSeparationTestFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define CheckBox0BasisHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define CheckBox1BasisHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
//#define FixOccupantCollisionBugHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define InterpretStateHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#endif
#define BlueHellHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define MenuBgCrashFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define DoorFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PointFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define PTFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define NodHarvesterGlitchFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line
#define ResultScreenGlitchFixHookAddr 0x000000 // For security and licensing purposes, an address has been hidden from this line

void XWISPlayerLimitBypassNaked1();
void XWISPlayerLimitBypassNaked2();

