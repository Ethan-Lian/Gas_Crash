#include "GameplayTags/GCTags.h"

namespace GCTags
{
	namespace InputTag
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Move, "InputTag.Move", "Tag for move input.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Look, "InputTag.Look", "Tag for look input.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Jump, "InputTag.Jump", "Tag for jump input.");

		namespace Ability
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Primary, "InputTag.Ability.Primary", "Tag for primary ability input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary, "InputTag.Ability.Secondary", "Tag for secondary ability input.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tertiary, "InputTag.Ability.Tertiary", "Tag for tertiary ability input.");
		}
	}

	namespace SetByCaller
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage, "GCTags.SetByCaller.Damage", "Unified damage magnitude for the ExecCalc pipeline.");
	}

	namespace DamageType
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Melee, "DamageType.Melee", "Semantic label: damage originated from a melee strike.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Projectile, "DamageType.Projectile", "Semantic label: damage originated from a projectile.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(AOE, "DamageType.AOE", "Semantic label: damage originated from an area-of-effect.");
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
		namespace player
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact, "GCTags.GCEvents.player.HitReact", "Tag for player hit react event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dead, "GCTags.GCEvents.player.Dead", "Tag for player dead state/event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(PrimaryAttack, "GCTags.GCEvents.player.PrimaryAttack", "Tag for player primary attack event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(SecondaryAttack, "GCTags.GCEvents.player.SecondaryAttack", "Tag for player secondary attack event.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(TertiaryAttack, "GCTags.GCEvents.player.TertiaryAttack", "Tag for player tertiary attack event.");
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

	namespace State
	{
		namespace CC
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Knockback, "State.CC.Knockback", "Enemy is currently under knockback crowd control.");
		}
	}

	namespace GameplayCue
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_DamageTaken_Melee, "GameplayCue.Character.DamageTaken.Melee", "GameplayCue tag for melee damage taken.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_DamageTaken_Projectile, "GameplayCue.Character.DamageTaken.Projectile", "GameplayCue tag for projectile damage taken.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_DamageTaken_SecondaryAOE, "GameplayCue.Character.DamageTaken.SecondaryAOE", "GameplayCue tag for secondary AOE damage taken.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary_Explosion, "GameplayCue.Secondary.Explosion", "GameplayCue tag for secondary explosion.");
	}

	namespace Cooldown
	{
		namespace Ability
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Primary, "Cooldown.Ability.Primary", "Cooldown tag for primary melee ability.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary, "Cooldown.Ability.Secondary", "Cooldown tag for secondary AOE ability.");
		}
	}
}
