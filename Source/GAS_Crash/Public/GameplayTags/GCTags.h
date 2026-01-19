#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Use NameSpace to avoid Naming Conflict
namespace GCTags
{
	//AbilityTag used to activate abilities
	namespace GCAbilities
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Secondary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tertiary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivateOnGiven);
	}
	//EventTag used to Send information
	namespace GCEvents
	{
		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
		}
	}
}