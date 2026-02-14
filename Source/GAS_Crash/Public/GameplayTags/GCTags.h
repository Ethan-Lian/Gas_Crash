#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Use NameSpace to avoid Naming Conflict
namespace GCTags
{
	namespace SetByCaller
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile)
	}
	
	//AbilityTag used to activate abilities
	namespace GCAbilities
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tertiary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivateOnGiven);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Respawn);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack);
		}
	}
	//EventTag used to Send information
	namespace GCEvents
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(KillScored);
		
		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
		}
	}
}