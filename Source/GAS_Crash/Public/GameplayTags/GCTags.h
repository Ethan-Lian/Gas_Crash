#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Use NameSpace to avoid Naming Conflict
namespace GCTags
{
	namespace InputTag
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Move);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Look);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);
	
		namespace Ability
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tertiary);
		}
	}
	
	namespace SetByCaller
	{
		// Unified damage tag for the damage execution pipeline.
		// Pair with DamageType tag on the GE Context for semantic type information.
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage)
	}

	// DamageType tags are semantic labels that describe "why" damage happened.
	// They live on the GE Context (not as SetByCaller) so downstream systems
	// (GameplayCue, UI) can branch without caring about data channels.
	namespace DamageType
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Melee)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(AOE)
	}
	
	//AbilityTag used to activate abilities
	namespace GCAbilities
	{
		namespace player
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tertiary);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Respawn);
		}
		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Respawn);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
		}

		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivateOnGiven);
	}
	//EventTag used to Send gameplay information
	namespace GCEvents
	{
		namespace player
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryAttack);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(SecondaryAttack);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(TertiaryAttack);
		}

		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
		}
	}

	namespace GCIdentity
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy);
	}

	namespace State
	{
		namespace CC
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Knockback)
		}
	}

	namespace GameplayCue
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_DamageTaken_Melee);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_DamageTaken_Projectile);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_DamageTaken_SecondaryAOE);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary_Explosion);
	}

	// ⭐⭐⭐ Interview Point:
	// Cooldown tags are GRANTED by the CooldownGE (Duration GE with GrantedTags).
	// CheckCooldown() queries ASC for these tags — if present, the ability is still on cooldown.
	// Each ability needs a unique cooldown tag so cooldowns don't interfere with each other.
	namespace Cooldown
	{
		namespace Ability
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary)
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary)
		}
	}
}
