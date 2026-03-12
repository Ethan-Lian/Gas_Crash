#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Use NameSpace to avoid Naming Conflict
namespace GCTags
{
	namespace SetByCaller
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Melee)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SecondaryAOEAbility)
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
	//EventTag used to Send information
	namespace GCEvents
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(KillScored);
		
		namespace player
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryAttack);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(SecondaryAttack);
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
}
