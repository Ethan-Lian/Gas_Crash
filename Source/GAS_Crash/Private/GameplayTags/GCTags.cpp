#include "GameplayTags/GCTags.h"

namespace GCTags
{
	namespace GCAbilities
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Primary,"GCTags.GCAbilities.Primary","Tag for the Primary Ability");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Secondary,"GCTags.GCAbilities.Secondary","Tag for the Secondary Ability");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tertiary,"GCTags.GCAbilities.Tertiary","Tag for the Tertiary Ability");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivateOnGiven,"GCTags.GCAbilities.ActivateOnGiven","Tag for Ability that should activate immediately once given");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death,"GCTags.GCAbilities.Death","Tag for the Death Ability");
		
		namespace Enemy
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack,"GCTags.GCAbilities.Enemy.Attack","Tag for the Enemy Attack Ability");
		}
	}
	
	namespace GCEvents
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(KillScored,"GCTags.Events.KillScored","Tag for the KillScored")
		
		namespace Enemy
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact,"GCTags.Events.Enemy.HitReact","Tag for the Enemy HitReact")
		}
	}
}