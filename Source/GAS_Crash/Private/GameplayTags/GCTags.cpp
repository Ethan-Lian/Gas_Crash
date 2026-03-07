#include "GameplayTags/GCTags.h"

namespace GCTags
{
	namespace SetByCaller
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Projectile, "GCTags.SetByCaller.Projectile", "Tag for SetByCaller projectile damage magnitude.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Melee, "GCTags.SetByCaller.Melee", "Tag for SetByCaller melee damage magnitude.");
	}

	namespace GCAbilities
	{
		namespace player
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Primary, "GCTags.GCAbilities.player.Primary", "Tag for player primary ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary, "GCTags.GCAbilities.player.Secondary", "Tag for player secondary ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tertiary, "GCTags.GCAbilities.player.Tertiary", "Tag for player tertiary ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death, "GCTags.GCAbilities.player.Death", "Tag for player death ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Respawn, "GCTags.GCAbilities.player.Respawn", "Tag for player respawn ability.");
		}

		namespace Enemy
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Respawn, "GCTags.GCAbilities.Enemy.Respawn", "Tag for enemy respawn ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack, "GCTags.GCAbilities.Enemy.Attack", "Tag for enemy attack ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death, "GCTags.GCAbilities.Enemy.Death", "Tag for enemy death ability.");
		}

		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivateOnGiven, "GCTags.GCAbilities.ActivateOnGiven", "Tag for abilities that auto-activate when granted.");
	}

	namespace GCEvents
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(KillScored, "GCTags.GCEvents.KillScored", "Tag for kill scored event.");

		namespace player
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact, "GCTags.GCEvents.player.HitReact", "Tag for player hit react event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dead, "GCTags.GCEvents.player.Dead", "Tag for player dead state/event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(PrimaryAttack, "GCTags.GCEvents.player.PrimaryAttack", "Tag for player primary attack event.");
		}

		namespace Enemy
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact, "GCTags.GCEvents.Enemy.HitReact", "Tag for enemy hit react event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dead, "GCTags.GCEvents.Enemy.Dead", "Tag for enemy dead state/event.");
		}
	}

	namespace GCIdentity
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player, "GCTags.GCIdentity.Player", "Tag to identify player characters.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Enemy, "GCTags.GCIdentity.Enemy", "Tag to identify enemy characters.");
	}
}
