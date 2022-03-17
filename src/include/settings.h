#pragma once
#include "Utils.h"

#include "debuffHandler.h"
#include "lib/robin_hood.h"
#include "lib/SimpleIni.h"


/*All the settings of Valhalla combat*/
class settings
{
public:

#pragma region StaminSettings
	static inline bool bUIAlert = true;
	static inline bool bNonCombatStaminaDebuff = true;
	
	
	static inline float fStaminaRegenMult = 5;
	static inline float fStaminaRegenLimit = 50;
	static inline float fCombatStaminaRegenMult = 1;
	static inline float fStaminaRegenDelay = 2;

	static inline bool bBlockStaminaToggle = true;
	static inline bool bGuardBreak = true;
	static inline float fBckShdStaminaMult_PC_Block_NPC = 1;
	static inline float fBckWpnStaminaMult_PC_Block_NPC = 1;

	static inline float fBckShdStaminaMult_NPC_Block_PC = 1; //stamina penalty mult for NPCs blockign a player hit with a shield
	static inline float fBckWpnStaminaMult_NPC_Block_PC = 1;

	static inline float fBckShdStaminaMult_NPC_Block_NPC = 1;
	static inline float fBckWpnStaminaMult_NPC_Block_NPC = 1;

	static inline bool bAttackStaminaToggle = true;
	static inline bool bBlockedHitRegenStamina = false;

	
	static inline float fMeleeCostLightMiss_Point = 30;
	static inline float fMeleeRewardLightHit_Percent = 0.2;
	static inline float fMeleeCostHeavyMiss_Percent = 0.333;
	static inline float fMeleeCostHeavyHit_Percent = 0.199;

	static inline uint32_t uExecutionKey = 35; //H for now
#pragma endregion
#pragma region PerfectBlockingSettings
	/*perfect blocking*/
	//static inline bool bPoiseCompatibility = false;

	static inline bool bPerfectBlockToggle = true;
	static inline bool bPerfectBlockScreenShake = true;
	static inline bool bPerfectBlockSFX = true;
	static inline bool bPerfectBlockVFX = true;
	static inline float fPerfectBlockTime = 0.33;
	static inline float fPerfectBlockCoolDownTime = 1;

#pragma endregion
#pragma region StunSettings

	/*Enable/disable stun and execution.*/
	static inline bool bStunToggle = true;
	static inline bool bStunMeterToggle = true;
	static inline float fStunParryMult = 1;
	static inline float fStunBashMult = 0.3;
	static inline float fStunPowerBashMult = 1;
	static inline float fStunNormalAttackMult = 0.3;
	static inline float fStunPowerAttackMult = 1.5;


	static inline float fStunUnarmedMult = 1.3;
	static inline float fStunDaggerMult = 0.5;
	static inline float fStunSwordMult = 0.9;
	static inline float fStunWarAxeMult = 1.1;
	static inline float fStunMaceMult = 1.1;
	static inline float fStunGreatSwordMult = 1.2;
	static inline float fStun2HBluntMult = 1.3;


	//execute
	/*Toggle for player to be executed.*/
	static inline bool bPlayerExecution = false;
	/*Toggle for player teammate to be executed.*/
	static inline bool bPlayerTeammateExecution = false;
	/*Toggle for essential NPCs to be executed.*/
	static inline bool bEssentialExecution = false;
	static inline bool bExecutionLimit = true; //NPCs with level higher than player cannot be executed unless under certain health threshold.
#pragma endregion
#pragma region Compatibility
	static inline bool bPoiseCompatibility = false;
#pragma endregion

	//TODO: add new settings

	static void readSettings();
	
private:
	static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
	static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);

};

namespace Utils
{
	/*tweaks the value of designated game setting
	@param gameSettingStr game setting to be tweaked.
	@param val desired float value of gamesetting.*/
	static void setGameSettingf(const char* settingStr, float val) {
		RE::Setting* setting = nullptr;
		RE::GameSettingCollection* _settingCollection = RE::GameSettingCollection::GetSingleton();
		setting = _settingCollection->GetSetting(settingStr);
		if (!setting) {
			INFO("invalid setting: {}", settingStr);
		}
		else {
			INFO("setting {} from {} to {}", settingStr, setting->GetFloat(), val);
			setting->data.f = val;
		}
	}

	/*a map of all game races' original stamina regen, in case player wants to tweak the stamina regen values again*/
	inline static robin_hood::unordered_map<RE::TESRace*, float> staminaRegenMap;

	/*multiplies stamina regen of every single race by MULT.
	@param mult multiplier for stamina regen.
	@param upperLimit upper limit for stamina regen.*/
	static void multStaminaRegen(float mult, float upperLimit) {
		for (auto& race : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESRace>()) {
			if (race && race->data.staminaRegen) {
				float staminaRegen;
				if (staminaRegenMap.find(race) == staminaRegenMap.end()) {
					staminaRegenMap[race] = race->data.staminaRegen;
					staminaRegen = staminaRegenMap[race] * mult;
				}
				else {
					staminaRegen = mult * staminaRegenMap[race];
				}
				if (staminaRegen > upperLimit) {
					staminaRegen = upperLimit;
				}
				race->data.staminaRegen = staminaRegen;
				INFO("setting stamina regen rate for race {} to {}.", race->GetName(), staminaRegen);
			}
		}
	}

}