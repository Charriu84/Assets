// cityAI.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvCityAI.h"
#include "CvGameAI.h"
#include "CvPlot.h"
#include "CvArea.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CyCity.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "FProfiler.h"

#include "CvDLLPythonIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
#include "BetterBTSAI.h"
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

#define BUILDINGFOCUS_FOOD					(1 << 1)
#define BUILDINGFOCUS_PRODUCTION			(1 << 2)
#define BUILDINGFOCUS_GOLD					(1 << 3)
#define BUILDINGFOCUS_RESEARCH				(1 << 4)
#define BUILDINGFOCUS_CULTURE				(1 << 5)
#define BUILDINGFOCUS_DEFENSE				(1 << 6)
#define BUILDINGFOCUS_HAPPY					(1 << 7)
#define BUILDINGFOCUS_HEALTHY				(1 << 8)
#define BUILDINGFOCUS_EXPERIENCE			(1 << 9)
#define BUILDINGFOCUS_MAINTENANCE			(1 << 10)
#define BUILDINGFOCUS_SPECIALIST			(1 << 11)
#define BUILDINGFOCUS_ESPIONAGE				(1 << 12)
#define BUILDINGFOCUS_BIGCULTURE			(1 << 13)
#define BUILDINGFOCUS_WORLDWONDER			(1 << 14)
#define BUILDINGFOCUS_DOMAINSEA				(1 << 15)
#define BUILDINGFOCUS_WONDEROK				(1 << 16)
#define BUILDINGFOCUS_CAPITAL				(1 << 17)



// Public Functions...

CvCityAI::CvCityAI()
{
	m_aiEmphasizeYieldCount = new int[NUM_YIELD_TYPES];
	m_aiEmphasizeCommerceCount = new int[NUM_COMMERCE_TYPES];
	m_bForceEmphasizeCulture = false;
	m_aiSpecialYieldMultiplier = new int[NUM_YIELD_TYPES];
	m_aiPlayerCloseness = new int[MAX_PLAYERS];

	m_pbEmphasize = NULL;

	AI_reset();
}


CvCityAI::~CvCityAI()
{
	AI_uninit();

	SAFE_DELETE_ARRAY(m_aiEmphasizeYieldCount);
	SAFE_DELETE_ARRAY(m_aiEmphasizeCommerceCount);
	SAFE_DELETE_ARRAY(m_aiSpecialYieldMultiplier);
	SAFE_DELETE_ARRAY(m_aiPlayerCloseness);
}


void CvCityAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
	AI_assignWorkingPlots();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      11/14/09                                jdog5000      */
/*                                                                                              */
/* City AI, Worker AI                                                                           */
/************************************************************************************************/
/* original bts code
	AI_updateWorkersNeededHere();
	
	AI_updateBestBuild();
*/
	AI_updateBestBuild();

	AI_updateWorkersNeededHere();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}


void CvCityAI::AI_uninit()
{
	SAFE_DELETE_ARRAY(m_pbEmphasize);
}


// FUNCTION: AI_reset()
// Initializes data members that are serialized.
void CvCityAI::AI_reset()
{
	int iI;

	AI_uninit();

	m_iEmphasizeAvoidGrowthCount = 0;
	m_iEmphasizeGreatPeopleCount = 0;
	m_bForceEmphasizeCulture = false;

	m_bAssignWorkDirty = false;
	m_bChooseProductionDirty = false;

	m_routeToCity.reset();

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiEmphasizeYieldCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiEmphasizeCommerceCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aiBestBuildValue[iI] = NO_BUILD;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aeBestBuild[iI] = NO_BUILD;
	}
	
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSpecialYieldMultiplier[iI] = 0;
	}
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiPlayerCloseness[iI] = 0;
	}
	m_iCachePlayerClosenessTurn = -1;
	m_iCachePlayerClosenessDistance = -1;
	
	m_iNeededFloatingDefenders = -1;
	m_iNeededFloatingDefendersCacheTurn = -1;

	m_iWorkersNeeded = 0;
	m_iWorkersHave = 0;

	FAssertMsg(m_pbEmphasize == NULL, "m_pbEmphasize not NULL!!!");
	FAssertMsg(GC.getNumEmphasizeInfos() > 0,  "GC.getNumEmphasizeInfos() is not greater than zero but an array is being allocated in CvCityAI::AI_reset");
	m_pbEmphasize = new bool[GC.getNumEmphasizeInfos()];
	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		m_pbEmphasize[iI] = false;
	}
}


void CvCityAI::AI_doTurn()
{
	PROFILE_FUNC();

	int iI;

	if (!isHuman())
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			setForceSpecialistCount(((SpecialistTypes)iI), 0);
		}
	}
	
    if (!isHuman())
	{
	    AI_stealPlots();
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      11/14/09                                jdog5000      */
/*                                                                                              */
/* City AI, Worker AI                                                                           */
/************************************************************************************************/
/* original bts code
	AI_updateWorkersNeededHere();
	
	AI_updateBestBuild();
*/
	AI_updateBestBuild();

	AI_updateWorkersNeededHere();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	AI_updateRouteToCity();

	if (isHuman())
	{
	    if (isProductionAutomated())
	    {
	        AI_doHurry();	        
	    }
		return;
	}
	
	AI_doPanic();

	AI_doDraft();

	AI_doHurry();

	AI_doEmphasize();
}


void CvCityAI::AI_assignWorkingPlots()
{
	PROFILE_FUNC();

	CvPlot* pHomePlot;
	int iI;
	
	if (0 != GC.getDefineINT("AI_SHOULDNT_MANAGE_PLOT_ASSIGNMENT"))
	{
		return;
	}

	// remove all assigned plots if we automated
	if (!isHuman() || isCitizensAutomated())
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			setWorkingPlot(iI, false);
		}
	}
	
	//update the special yield multiplier to be current
	AI_updateSpecialYieldMultiplier();
	
	// remove any plots we can no longer work for any reason
	verifyWorkingPlots();
	
	// if forcing specialists, try to make all future specialists of the same type
	bool bIsSpecialistForced = false;
	int iTotalForcedSpecialists = 0;

	// make sure at least the forced amount of specialists are assigned
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
		if (iForcedSpecialistCount > 0)
		{
			bIsSpecialistForced = true;
			iTotalForcedSpecialists += iForcedSpecialistCount;
		}
		
		if (!isHuman() || isCitizensAutomated() || (getSpecialistCount((SpecialistTypes)iI) < iForcedSpecialistCount))
		{
			setSpecialistCount(((SpecialistTypes)iI), iForcedSpecialistCount);
		}
	}
	
	// if we have more specialists of any type than this city can have, reduce to the max
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (!isSpecialistValid((SpecialistTypes)iI))
		{
			if (getSpecialistCount((SpecialistTypes)iI) > getMaxSpecialistCount((SpecialistTypes)iI))
			{
				setSpecialistCount(((SpecialistTypes)iI), getMaxSpecialistCount((SpecialistTypes)iI));
			}
		}
	}
	
	// always work the home plot (center)
	pHomePlot = getCityIndexPlot(CITY_HOME_PLOT);
	if (pHomePlot != NULL)
	{
		setWorkingPlot(CITY_HOME_PLOT, ((getPopulation() > 0) && canWork(pHomePlot)));
	}
	
	// keep removing the worst citizen until we are not over the limit
	while (extraPopulation() < 0)
	{
		if (!AI_removeWorstCitizen())
		{
			FAssert(false);
			break;
		}
	}
	
	// extraSpecialists() is less than extraPopulation()
	FAssertMsg(extraSpecialists() >= 0, "extraSpecialists() is expected to be non-negative (invalid Index)");
	
	// do we have population unassigned
	while (extraPopulation() > 0)
	{
		// (AI_addBestCitizen now handles forced specialist logic)
		if (!AI_addBestCitizen(/*bWorkers*/ true, /*bSpecialists*/ true))
		{
			break;
		}
	}

	// if forcing specialists, assign any other specialists that we must place based on forced specialists
	int iInitialExtraSpecialists = extraSpecialists();
	int iExtraSpecialists = iInitialExtraSpecialists;
	if (bIsSpecialistForced && iExtraSpecialists > 0)
	{
		FAssertMsg(iTotalForcedSpecialists > 0, "zero or negative total forced specialists");
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (isSpecialistValid((SpecialistTypes)iI, 1))
			{
				int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
				if (iForcedSpecialistCount > 0)
				{
					int iSpecialistCount = getSpecialistCount((SpecialistTypes)iI);
					int iMaxSpecialistCount = getMaxSpecialistCount((SpecialistTypes)iI);

					int iSpecialistsToAdd = ((iInitialExtraSpecialists * iForcedSpecialistCount) + (iTotalForcedSpecialists/2)) / iTotalForcedSpecialists;
					if (iExtraSpecialists < iSpecialistsToAdd)
					{
						iSpecialistsToAdd = iExtraSpecialists;
					}
					
					iSpecialistCount += iSpecialistsToAdd;
					iExtraSpecialists -= iSpecialistsToAdd;

					// if we cannot fit that many, then add as many as we can
					if (iSpecialistCount > iMaxSpecialistCount && !GET_PLAYER(getOwnerINLINE()).isSpecialistValid((SpecialistTypes)iI))
					{
						iExtraSpecialists += iSpecialistCount - iMaxSpecialistCount;
						iSpecialistCount = iMaxSpecialistCount;
					}

					setSpecialistCount((SpecialistTypes)iI, iSpecialistCount);
				}
			}
		}
	}
	FAssertMsg(iExtraSpecialists >= 0, "added too many specialists");

	// if we still have population to assign, assign specialists
	while (extraSpecialists() > 0)
	{
		if (!AI_addBestCitizen(/*bWorkers*/ false, /*bSpecialists*/ true))
		{
			break;
		}
	}
	
	// if automated, look for better choices than the current ones
	if (!isHuman() || isCitizensAutomated())
	{
		AI_juggleCitizens();
	}
	
	// at this point, we should not be over the limit
	FAssert((getWorkingPopulation() + getSpecialistPopulation()) <= (totalFreeSpecialists() + getPopulation()));

	AI_setAssignWorkDirty(false);

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
	}
}


void CvCityAI::AI_updateAssignWork()
{
	if (AI_isAssignWorkDirty())
	{
		AI_assignWorkingPlots();
	}
}


bool CvCityAI::AI_avoidGrowth()
{
	PROFILE_FUNC();

	if (AI_isEmphasizeAvoidGrowth())
	{
		return true;
	}
	
	if (isFoodProduction())
	{
		return true;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/14/10                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
/* original code
	if (!AI_isEmphasizeYield(YIELD_FOOD) && !AI_isEmphasizeGreatPeople())
*/
	// AI should avoid growth when it has angry citizens, even if emphasizing great people
	if( !(isHuman()) || (!AI_isEmphasizeYield(YIELD_FOOD) && !AI_isEmphasizeGreatPeople()) )
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	{
		int iExtra = (isHuman()) ? 0 : 1;
		int iHappinessLevel = happyLevel() - unhappyLevel(iExtra);
		
		// ignore military unhappy, since we assume it will be fixed quickly, grow anyway
		if (getMilitaryHappinessUnits() == 0)
		{
			iHappinessLevel += ((GC.getDefineINT("NO_MILITARY_PERCENT_ANGER") * (getPopulation() + 1)) / GC.getPERCENT_ANGER_DIVISOR());
		}
		

		// if we can pop rush, we want to grow one over the cap
		if (GET_PLAYER(getOwnerINLINE()).canPopRush())
		{
			iHappinessLevel++;
		}

		
		// if we have angry citizens
		if (iHappinessLevel < 0)
		{
			return true;
		}
	}

	return false;
}


bool CvCityAI::AI_ignoreGrowth()
{
	PROFILE_FUNC();

	if (!AI_isEmphasizeYield(YIELD_FOOD) && !AI_isEmphasizeGreatPeople())
	{
		if (!AI_foodAvailable((isHuman()) ? 0 : 1))
		{
			return true;
		}
	}

	return false;
}


int CvCityAI::AI_specialistValue(SpecialistTypes eSpecialist, bool bAvoidGrowth, bool bRemove)
{
	PROFILE_FUNC();

	short aiYields[NUM_YIELD_TYPES];
	int iTempValue;
	int iGreatPeopleRate;
	int iValue;
	int iI, iJ;
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();
    
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = GET_PLAYER(getOwnerINLINE()).specialistYield(eSpecialist, ((YieldTypes)iI));
	}

	short int aiCommerceYields[NUM_COMMERCE_TYPES];
	
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceYields[iI] = GET_PLAYER(getOwnerINLINE()).specialistCommerce(eSpecialist, ((CommerceTypes)iI));
	}
	
	iValue = AI_yieldValue(aiYields, aiCommerceYields, bAvoidGrowth, bRemove);

	iGreatPeopleRate = GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();

	int iEmphasisCount = 0;
	if (iGreatPeopleRate != 0)
	{
		int iGPPValue = 4;
		if (AI_isEmphasizeGreatPeople())
		{
			iGPPValue = isHuman() ? 30 : 20;
		}
		else 
		{
			if (AI_isEmphasizeYield(YIELD_COMMERCE))
			{
				iGPPValue = 2;
				iEmphasisCount++;			
			}
			if (AI_isEmphasizeYield(YIELD_PRODUCTION))
			{
				iGPPValue = 1;
				iEmphasisCount++;
			}
			if (AI_isEmphasizeYield(YIELD_FOOD))
			{
				iGPPValue = 1;
				iEmphasisCount++;
			}
		}
		
		//iGreatPeopleRate = ((iGreatPeopleRate * getTotalGreatPeopleRateModifier()) / 100);
		// UnitTypes iGreatPeopleType = (UnitTypes)GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
		
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/06/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
		// Scale up value for civs/civics with bonuses
		iGreatPeopleRate *= (100 + GET_PLAYER(getOwnerINLINE()).getGreatPeopleRateModifier());
		iGreatPeopleRate /= 100;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

		iTempValue = (iGreatPeopleRate * iGPPValue);
		
//		if (isHuman() && (getGreatPeopleUnitRate(iGreatPeopleType) == 0)
//			&& (getForceSpecialistCount(eSpecialist) == 0) && !AI_isEmphasizeGreatPeople())
//		{
//			iTempValue -= (iGreatPeopleRate * 4);			
//		}

		if (!isHuman() || AI_isEmphasizeGreatPeople())
		{
			int iProgress = getGreatPeopleProgress();
			if (iProgress > 0)
			{
				int iThreshold = GET_PLAYER(getOwnerINLINE()).greatPeopleThreshold();
				iTempValue += ((iGreatPeopleRate * (isHuman() ? 1 : 4) * iGPPValue * iProgress * iProgress) / (iThreshold * iThreshold));
			}
		}
		
		int iCurrentEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
		int iTotalEras = GC.getNumEraInfos();
		
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
		if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
		{
			int iUnitClass = GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
			FAssert(iUnitClass != NO_UNITCLASS);
/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/************************************************************************************************/
            if (iUnitClass != NO_UNITCLASS)
			{
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			
			UnitTypes eGreatPeopleUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitClass);
			if (eGreatPeopleUnit != NO_UNIT)
			{
				CvUnitInfo& kUnitInfo = GC.getUnitInfo(eGreatPeopleUnit);
				if (kUnitInfo.getGreatWorkCulture() > 0)
				{
					iTempValue += kUnitInfo.getGreatWorkCulture() / ((GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3)) ? 200 : 350);
				}
			}
/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/************************************************************************************************/
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

        if (!isHuman() && (iCurrentEra <= ((iTotalEras * 2) / 3)))
        {
            // try to spawn a prophet for any shrines we have yet to build
            bool bNeedProphet = false;
            int iBestSpreadValue = 0;

			
			for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
            {
                ReligionTypes eReligion = (ReligionTypes) iJ;

                if (isHolyCity(eReligion) && !hasShrine(eReligion)
                	&& ((iCurrentEra < iTotalEras / 2) || GC.getGameINLINE().countReligionLevels(eReligion) >= 10))
                {
					CvCivilizationInfo* pCivilizationInfo = &GC.getCivilizationInfo(getCivilizationType());
					
					int iUnitClass = GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
                    FAssert(iUnitClass != NO_UNITCLASS);
/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/************************************************************************************************/
                    if (iUnitClass != NO_UNITCLASS)
					{
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					UnitTypes eGreatPeopleUnit = (UnitTypes) pCivilizationInfo->getCivilizationUnits(iUnitClass);
					if (eGreatPeopleUnit != NO_UNIT)
					{
						// note, for normal XML, this count will be one (there is only 1 shrine building for each religion)
						int	shrineBuildingCount = GC.getGameINLINE().getShrineBuildingCount(eReligion);
						for (int iI = 0; iI < shrineBuildingCount; iI++)
						{
							int eBuilding = (int) GC.getGameINLINE().getShrineBuilding(iI, eReligion);
							
							// if this unit builds or forceBuilds this building
							if (GC.getUnitInfo(eGreatPeopleUnit).getBuildings(eBuilding) || GC.getUnitInfo(eGreatPeopleUnit).getForceBuildings(eBuilding))
							{
								bNeedProphet = true;
								iBestSpreadValue = std::max(iBestSpreadValue, GC.getGameINLINE().countReligionLevels(eReligion));
							}
						}
					}
/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/************************************************************************************************/
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				}
			}

			if (bNeedProphet)
            {
                iTempValue += ((iGreatPeopleRate * iBestSpreadValue));
            }
		}
		
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/06/09                                jdog5000      */
/*                                                                                              */
/* Bugfix, City AI                                                                              */
/************************************************************************************************/
/* original BTS code
		iTempValue *= 100;
*/		
		// Scale up value for civs/civics with bonuses
		iTempValue *= getTotalGreatPeopleRateModifier();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		iTempValue /= GET_PLAYER(getOwnerINLINE()).AI_averageGreatPeopleMultiplier();
		
		iTempValue /= (1 + iEmphasisCount);
		iValue += iTempValue;
	}
	else
	{
		SpecialistTypes eGenericCitizen = (SpecialistTypes) GC.getDefineINT("DEFAULT_SPECIALIST");
		
		// are we the generic specialist?
		if (eSpecialist == eGenericCitizen)
		{
			iValue *= 60;
			iValue /= 100;
		}
	}
	
	int iExperience = GC.getSpecialistInfo(eSpecialist).getExperience();
	if (0 != iExperience)
	{
		int iProductionRank = findYieldRateRank(YIELD_PRODUCTION);
		int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

		iValue += (iExperience * ((iHasMetCount > 0) ? 4 : 2));
		if (iProductionRank <= iNumCities/2 + 1)
		{
			iValue += iExperience *  4;
		}
		iValue += ((getMilitaryProductionModifier() * iExperience * 8) / 100);
	}

	return (iValue * 100);
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/22/09                                jdog5000      */
/*                                                                                              */
/* City AI, War Strategy AI                                                                     */
/************************************************************************************************/
void CvCityAI::AI_chooseProduction()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	UnitTypes eProductionUnit;
	bool bWasFoodProduction;
	bool bHasMetHuman;
	bool bLandWar;
	bool bAssault;
	bool bDefenseWar;
	bool bPrimaryArea;
	bool bFinancialTrouble;
	bool bDanger;
	bool bChooseUnit;
	int iProductionRank;
	int iCulturePressure;

	bDanger = AI_isDanger();
	
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	if (isProduction())
	{
		if (getProduction() > 0)
		{

			if ((getProductionUnitAI() == UNITAI_SETTLE) && kPlayer.AI_isFinancialTrouble())
			{
				
			}
			//if we are killing our growth to train this, then finish it.
			else if (!bDanger && isFoodProduction())
			{
				if ((area()->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE))
				{
					return;
				}
			}
			// if less than 3 turns left, keep building current item
			else if (getProductionTurnsLeft() <= 3)
			{
				return;
			}
			
			
			// if building a combat unit, and we have no defenders, keep building it
			eProductionUnit = getProductionUnit();
			if (eProductionUnit != NO_UNIT)
			{
				if (plot()->getNumDefenders(getOwnerINLINE()) == 0)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						return;
					}
				}
			}
			
			// if we are building a wonder, do not cancel, keep building it (if no danger)
			BuildingTypes eProductionBuilding = getProductionBuilding();
			if (!bDanger && eProductionBuilding != NO_BUILDING && 
				isLimitedWonderClass((BuildingClassTypes) GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))
			{
				return;
			}
		}

		clearOrderQueue();
	}
	
	if (GET_PLAYER(getOwner()).isAnarchy())
	{
		return;
	}
	
	// only clear the dirty bit if we actually do a check, multiple items might be queued
	AI_setChooseProductionDirty(false);

	// allow python to handle it
	CyCity* pyCity = new CyCity(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCity));	// pass in city class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_chooseProduction", argsList.makeFunctionArgs(), &lResult);
	delete pyCity;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return;
	}

    if (isHuman() && isProductionAutomated())
    {
        AI_buildGovernorChooseProduction();
        return;
    }
	
	CvArea* pArea = area();
	pWaterArea = waterArea(true);
	bool bMaybeWaterArea = false;
	bool bWaterDanger = false;
    
	if (pWaterArea != NULL)
	{
		bMaybeWaterArea = true;
		if (!GET_TEAM(getTeam()).AI_isWaterAreaRelevant(pWaterArea))
		{
			pWaterArea = NULL;
		}

		bWaterDanger = kPlayer.AI_getWaterDanger(plot(), 4) > 0;
	}

	bWasFoodProduction = isFoodProduction();
	bHasMetHuman = GET_TEAM(getTeam()).hasMetHuman();
	bLandWar = ((pArea->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (pArea->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bDefenseWar = (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
	bool bAssaultAssist = (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT_ASSIST);
	bAssault = bAssaultAssist || (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT) || (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT_MASSING);
	bPrimaryArea = kPlayer.AI_isPrimaryArea(pArea);
	bFinancialTrouble = kPlayer.AI_isFinancialTrouble();
	iCulturePressure = AI_calculateCulturePressure();
	int iNumCitiesInArea = pArea->getCitiesPerPlayer(getOwnerINLINE());
	bool bImportantCity = false; //be very careful about setting this.
	bool bBigCultureCity = false;
	int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
    int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	int iWarSuccessRatio = GET_TEAM(getTeam()).AI_getWarSuccessCapitulationRatio();
	int iEnemyPowerPerc = GET_TEAM(getTeam()).AI_getEnemyPowerPercent(true);
	int iWarTroubleThreshold = 0;

	if( bLandWar && iWarSuccessRatio < 30 )
	{
		iWarTroubleThreshold = std::max(3,(-iWarSuccessRatio/8));
	}

	if( !bLandWar && !bAssault && GET_TEAM(getTeam()).isAVassal() )
	{
		bLandWar = GET_TEAM(getTeam()).isMasterPlanningLandWar(area());

		if( !bLandWar )
		{
			bAssault = GET_TEAM(getTeam()).isMasterPlanningSeaWar(area());
		}
	}

    bool bGetBetterUnits = kPlayer.AI_isDoStrategy(AI_STRATEGY_GET_BETTER_UNITS);
    bool bAggressiveAI = GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI);
    bool bAlwaysPeace = GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE);

	int iUnitCostPercentage = (kPlayer.calculateUnitCost() * 100) / std::max(1, kPlayer.calculatePreInflatedCosts());
	int iWaterPercent = AI_calculateWaterWorldPercent();
	
	int iBuildUnitProb = AI_buildUnitProb();
    
    int iExistingWorkers = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_WORKER);
    int iNeededWorkers = kPlayer.AI_neededWorkers(pArea);
	// Sea worker need independent of whether water area is militarily relevant
	int iNeededSeaWorkers = (bMaybeWaterArea) ? AI_neededSeaWorkers() : 0;
	int iExistingSeaWorkers = (waterArea(true) != NULL) ? kPlayer.AI_totalWaterAreaUnitAIs(waterArea(true), UNITAI_WORKER_SEA) : 0;


    int iTargetCulturePerTurn = AI_calculateTargetCulturePerTurn();
    
    int iAreaBestFoundValue;
    int iNumAreaCitySites = kPlayer.AI_getNumAreaCitySites(getArea(), iAreaBestFoundValue);
    
    int iWaterAreaBestFoundValue = 0;
	CvArea* pWaterSettlerArea = pWaterArea;
	if( pWaterSettlerArea == NULL )
	{
		pWaterSettlerArea = GC.getMap().findBiggestArea(true);

		if( GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterSettlerArea, UNITAI_SETTLER_SEA) == 0 )
		{
			pWaterSettlerArea = NULL;
		}
	}
    int iNumWaterAreaCitySites = (pWaterSettlerArea == NULL) ? 0 : kPlayer.AI_getNumAdjacentAreaCitySites(pWaterSettlerArea->getID(), getArea(), iWaterAreaBestFoundValue);
    int iNumSettlers = kPlayer.AI_totalUnitAIs(UNITAI_SETTLE);
    
    bool bIsCapitalArea = false;
	int iNumCapitalAreaCities = 0;
    if (kPlayer.getCapitalCity() != NULL)
    {
		iNumCapitalAreaCities = kPlayer.getCapitalCity()->area()->getCitiesPerPlayer(getOwnerINLINE());
    	if (getArea() == kPlayer.getCapitalCity()->getArea())
    	{
    		bIsCapitalArea = true;
    	}
    }
        
    int iMaxSettlers = 0;
    if (!bFinancialTrouble)
    {
     	iMaxSettlers= std::min((kPlayer.getNumCities() + 1) / 2, iNumAreaCitySites + iNumWaterAreaCitySites);
     	if (bLandWar || bAssault)
     	{
     		iMaxSettlers = (iMaxSettlers + 2) / 3;
     	}
    }

    
    bool bChooseWorker = false;
    
	int iEconomyFlags = 0;
	iEconomyFlags |= BUILDINGFOCUS_PRODUCTION;
	iEconomyFlags |= BUILDINGFOCUS_GOLD;
	iEconomyFlags |= BUILDINGFOCUS_RESEARCH;
	iEconomyFlags |= BUILDINGFOCUS_MAINTENANCE;
	iEconomyFlags |= BUILDINGFOCUS_HAPPY;
	iEconomyFlags |= BUILDINGFOCUS_HEALTHY;
	if (AI_isEmphasizeGreatPeople())
	{
		iEconomyFlags |= BUILDINGFOCUS_SPECIALIST;
	}
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		iEconomyFlags |= BUILDINGFOCUS_ESPIONAGE;
	}

	if (iNumCitiesInArea > 2)
	{
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
		{
			if (iCultureRateRank <= iCulturalVictoryNumCultureCities + 1)
			{
				bBigCultureCity = true;

				// if we do not have enough cities, then the highest culture city will not get special attention
				if (iCultureRateRank > 1 || (kPlayer.getNumCities() > (iCulturalVictoryNumCultureCities + 1)))
				{
					if ((((iNumAreaCitySites + iNumWaterAreaCitySites) > 0) && (kPlayer.getNumCities() < 6)) && (GC.getGameINLINE().getSorenRandNum(2, "AI Less Culture More Expand") == 0))
					{
						bImportantCity = false;
					}
					else
					{
						bImportantCity = true;
					}
				}
            }
        }
	}

	// Free experience for various unit domains
	int iFreeLandExperience = getSpecialistFreeExperience() + getDomainFreeExperience(DOMAIN_LAND);
	int iFreeSeaExperience = getSpecialistFreeExperience() + getDomainFreeExperience(DOMAIN_SEA);
	int iFreeAirExperience = getSpecialistFreeExperience() + getDomainFreeExperience(DOMAIN_AIR);

	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	if( gCityLogLevel >= 3 ) logBBAI("      City %S pop %d considering new production: iProdRank %d, iBuildUnitProb %d", getName().GetCString(), getPopulation(), iProductionRank, iBuildUnitProb);

	// -------------------- BBAI Notes -------------------------
	// Start special circumstances

	// -------------------- BBAI Notes -------------------------
	// Barbarian city build priorities
	if (isBarbarian())
	{
		if (!AI_isDefended(plot()->plotCount(PUF_isUnitAIType, UNITAI_ATTACK, -1, getOwnerINLINE()))) // XXX check for other team's units?
		{
			if (AI_chooseDefender())
			{
				return;
			}

			if (AI_chooseUnit(UNITAI_ATTACK))
			{
				return;
			}
		}
		
		if (!bDanger && (2*iExistingWorkers < iNeededWorkers) && (AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
		{
			if( getPopulation() > 1 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
			{
				if (AI_chooseUnit(UNITAI_WORKER))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose worker 1", getName().GetCString());
					return;
				}
			}			
		}

		if (!bDanger && !bWaterDanger && (iNeededSeaWorkers > 0))
		{
			if (AI_chooseUnit(UNITAI_WORKER_SEA))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose worker sea 1", getName().GetCString());
				return;
			}
		}
		
		iBuildUnitProb += (3 * iFreeLandExperience);
		
		bool bRepelColonists = false;
		if( area()->getNumCities() > area()->getCitiesPerPlayer(BARBARIAN_PLAYER) + 2 )
		{
			if( area()->getCitiesPerPlayer(BARBARIAN_PLAYER) > area()->getNumCities()/3 )
			{
				// New world scenario with invading colonists ... fight back!
				bRepelColonists = true;
				iBuildUnitProb += 8*(area()->getNumCities() - area()->getCitiesPerPlayer(BARBARIAN_PLAYER));
			}
		}

		bChooseUnit = false;
		if (!bDanger && GC.getGameINLINE().getSorenRandNum(100, "AI Build Unit Production") > iBuildUnitProb)
		{
			
			int iBarbarianFlags = 0;
			if( getPopulation() < 4 ) iBarbarianFlags |= BUILDINGFOCUS_FOOD;
			iBarbarianFlags |= BUILDINGFOCUS_PRODUCTION;
			iBarbarianFlags |= BUILDINGFOCUS_EXPERIENCE;
			if( getPopulation() > 3 ) iBarbarianFlags |= BUILDINGFOCUS_DEFENSE;
			
			if (AI_chooseBuilding(iBarbarianFlags, 15))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb AI_chooseBuilding with flags and iBuildUnitProb = %d", getName().GetCString(), iBuildUnitProb);
				return;
			}

			if( GC.getGameINLINE().getSorenRandNum(100, "AI Build Unit Production") > iBuildUnitProb)
			{
				if (AI_chooseBuilding())
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb AI_chooseBuilding without flags and iBuildUnitProb = %d", getName().GetCString(), iBuildUnitProb);
					return;
				}
			}
		}
		
		if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ASSAULT_SEA, -1, getOwnerINLINE()) > 0)
		{
			if (AI_chooseUnit(UNITAI_ATTACK_CITY))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose attack city for transports", getName().GetCString());
				return;
			}
		}
		
		if (!bDanger && (pWaterArea != NULL) && (iWaterPercent > 30))
		{
			if (GC.getGameINLINE().getSorenRandNum(3, "AI Coast Raiders!") == 0)
			{
				if (kPlayer.AI_totalUnitAIs(UNITAI_ASSAULT_SEA) <= (1 + kPlayer.getNumCities() / 2))
				{
					if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose transport", getName().GetCString());
						return;
					}
				}
			}
			if (GC.getGameINLINE().getSorenRandNum(110, "AI arrrr!") < (iWaterPercent + 10))
			{
				if (kPlayer.AI_totalUnitAIs(UNITAI_PIRATE_SEA) <= kPlayer.getNumCities())
				{
					if (AI_chooseUnit(UNITAI_PIRATE_SEA))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose pirate", getName().GetCString());
						return;
					}
				}
				
				if (kPlayer.AI_totalAreaUnitAIs(pWaterArea, UNITAI_ATTACK_SEA) < iNumCitiesInArea)
				{
					if (AI_chooseUnit(UNITAI_ATTACK_SEA))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose attack sea", getName().GetCString());
						return;
					}
				}
			}
		}

		if (GC.getGameINLINE().getSorenRandNum(2, "Barb worker") == 0)
		{
			if (!bDanger && (iExistingWorkers < iNeededWorkers) && (AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
			{
				if( getPopulation() > 1 )
				{
					if (AI_chooseUnit(UNITAI_WORKER))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses barb choose worker 2", getName().GetCString());
						return;
					}
				}			
			}
		}

		UnitTypeWeightArray barbarianTypes;
		barbarianTypes.push_back(std::make_pair(UNITAI_ATTACK, 125));
		barbarianTypes.push_back(std::make_pair(UNITAI_ATTACK_CITY, (bRepelColonists ? 100 : 50)));
		barbarianTypes.push_back(std::make_pair(UNITAI_COUNTER, 100));
		barbarianTypes.push_back(std::make_pair(UNITAI_CITY_DEFENSE, 50));

		if (AI_chooseLeastRepresentedUnit(barbarianTypes))
		{
			return;
		}
		
		if (AI_chooseUnit())
		{
			return;
		}
		
		return;
	}
	
	// if we need to pop borders, then do that immediately if we have drama and can do it
	if ((iTargetCulturePerTurn > 0) && (getCultureLevel() <= (CultureLevelTypes) 1))
	{
        if (AI_chooseProcess(COMMERCE_CULTURE))
        {
            return;
        }
	}

	if (isOccupation())
	{
		// pick granary or lighthouse, any duration
		if (AI_chooseBuilding(BUILDINGFOCUS_FOOD))
		{
			return;
		}

		// try picking forge, etc, any duration
		if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION))
		{
			return;
		}
		
		// just pick any building, any duration
		if (AI_chooseBuilding())
		{
			return;
		}
	}

	if (plot()->getNumDefenders(getOwnerINLINE()) == 0) // XXX check for other team's units?
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses no defenders", getName().GetCString());

		if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
		{
			return;
		}
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
		if (kPlayer.getNumCities() <= 3)
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE, -1, true))
			{
				return;
			}
		}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			return;
		}

		if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
		{
			return;
		}

/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
		if (AI_chooseUnit(UNITAI_RESERVE))
		{
			return;
		}

//Fuyu bIgnoreNotUnitAIs
		if (kPlayer.getNumCities() > 3 && iNumCitiesInArea <= 3)
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE, -1, true))
			{
				return;
			}
		}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

		if (AI_chooseUnit(UNITAI_ATTACK))
		{
			return;
		}
	}

	if( kPlayer.isStrike() )
	{
		// pick granary or lighthouse, any duration
		int iStrikeFlags = 0;
		iStrikeFlags |= BUILDINGFOCUS_GOLD;
		iStrikeFlags |= BUILDINGFOCUS_MAINTENANCE;

		if(AI_chooseBuilding(iStrikeFlags))
		{
			return;
		}

		// try picking forge, etc, any duration
		if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION))
		{
			return;
		}
		
		// just pick any building, any duration
		if (AI_chooseBuilding())
		{
			return;
		}
	}

	// So what's the right detection of defense which works in early game too?
	// Fuyu: The right way is to 1) queue the warriors with UNITAI_CITY_DEFENCE or UNITAI_RESERVE,
	// then 2) to detect defenders with plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) - compare AI_isDefended()
	int iPlotSettlerCount = (iNumSettlers == 0) ? 0 : plot()->plotCount(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE());
	int iPlotCityDefenderCount = plot()->plotCount(PUF_isUnitAIType, UNITAI_CITY_DEFENSE, -1, getOwnerINLINE());
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
	int iPlotOtherCityAICount = plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) - iPlotCityDefenderCount;
	if( kPlayer.getCurrentEra() == 0 )
	{
		// Warriors are blocked from UNITAI_CITY_DEFENSE, in early game this confuses AI city building
		if( kPlayer.AI_totalUnitAIs(UNITAI_CITY_DEFENSE) <= kPlayer.getNumCities() + iNumSettlers )
		{
			if( kPlayer.AI_bestCityUnitAIValue(UNITAI_CITY_DEFENSE, this) == 0 )
			{
				iPlotCityDefenderCount += iPlotOtherCityAICount;
				iPlotOtherCityAICount = 0;
				if (iPlotCityDefenderCount == 0)
				{
					iPlotCityDefenderCount = plot()->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isDomainType, DOMAIN_LAND);
				}
			}
		}
	}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

	//minimal defense.
	if (iPlotCityDefenderCount <= iPlotSettlerCount)
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S needs escort for existing settler", getName().GetCString());
		if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
		{
			// BBAI TODO: Does this work right after settler is built???
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses escort existing settler 1 defense", getName().GetCString());
			return;
		}

/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
		if (iNumCitiesInArea <= 3)
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE, -1, true))
			{
				// Yes it probably works but it should never happen in the first place
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses escort existing settler 1 defense (IgnoreNotUnitAIs)", getName().GetCString());
				return;
			}
		}

		if (iPlotCityDefenderCount + iPlotOtherCityAICount <= iPlotSettlerCount)
		{

			if (AI_chooseUnit(UNITAI_ATTACK))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses escort existing settler 1 attack", getName().GetCString());
				return;
			}

			if (AI_chooseUnit(UNITAI_CITY_COUNTER))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses escort existing settler 1 city counter", getName().GetCString());
				return;
			}

			if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses escort existing settler 1 city special", getName().GetCString());
				return;
			}

			if (AI_chooseUnit(UNITAI_RESERVE))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses escort existing settler 1 reserve", getName().GetCString());
				return;
			}

		}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/
	}
    
	if (((iTargetCulturePerTurn > 0) || (getPopulation() > 5)) && (getCommerceRate(COMMERCE_CULTURE) == 0))
	{
		if( !(kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE)) )
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30))
			{
				return;
			}
		}
	}

	if( !(bDefenseWar && iWarSuccessRatio < -50) && !bDanger )
	{
		if ((iExistingWorkers == 0))
		{
			int iLandBonuses = AI_countNumImprovableBonuses(true, kPlayer.getCurrentResearch());
			if ((iLandBonuses > 1) || (getPopulation() > 3 && iNeededWorkers > 0))
			{
				if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 1", getName().GetCString());
					return;
				}
				bChooseWorker = true;
			}

			if (!bWaterDanger && (iNeededSeaWorkers > iExistingSeaWorkers) && (getPopulation() < 3))
			{
				if (AI_chooseUnit(UNITAI_WORKER_SEA))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker sea 1", getName().GetCString());
					return;
				}
			}

			if (iLandBonuses >= 1  && getPopulation() > 1)
    		{
				if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 2", getName().GetCString());
					return;
				}
				bChooseWorker = true;
    		}
		}
	}

	// Early game worker logic
	if( isCapital() && (GC.getGame().getElapsedGameTurns() < ((30 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100)))
	{
		if( !bDanger && !(kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE)) )
		{	
			if (!bWaterDanger && (getPopulation() <= 2) && (iNeededSeaWorkers > 0))
			{
				if (iExistingSeaWorkers == 0)
				{
					// Build workboat first since it doesn't stop growth
					if (AI_chooseUnit(UNITAI_WORKER_SEA))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker sea 1a", getName().GetCString());
						return;
					}
				}
			}

			if( iExistingWorkers == 0 && AI_totalBestBuildValue(area()) > 0 /*Fuyu: anything bigger than 0 is ok*/ )
			{
				if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 1a", getName().GetCString());
					return;
				}
				bChooseWorker = true;
			}
/********************************************************************************/
/* 	Build more early sea workers								Fuyu		    */
/********************************************************************************/
			else
			{
				if (!bWaterDanger && (getPopulation() <= 4) && (iNeededSeaWorkers > 0) && (happyLevel() - unhappyLevel(1)) > 0)
				{
					if (iExistingSeaWorkers == 0)
					{
						if (AI_chooseUnit(UNITAI_WORKER_SEA))
						{
							if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker sea 1b", getName().GetCString());
							return;
						}
					}
				}
			}
/********************************************************************************/
/* 	Build more early sea workers								END			    */
/********************************************************************************/
		}
	}

	if ( kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3) )
	{
        if ((goodHealth() - badHealth(true, 0)) < 1)
		{
			if ( AI_chooseBuilding(BUILDINGFOCUS_HEALTHY, 20, 0, (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION4) ? 50 : 20)) )
			{
				return;
			}
		}
	}

	if( GET_TEAM(getTeam()).isAVassal() && GET_TEAM(getTeam()).isCapitulated() )
	{
		if( !bLandWar )
		{
			if ((goodHealth() - badHealth(true, 0)) < 1)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_HEALTHY, 30, 0, 3*getPopulation()))
				{
					return;
				}
			}

			if ((getPopulation() > 3) && (getCommerceRate(COMMERCE_CULTURE) < 5))
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30, 0 + 3*iWarTroubleThreshold, 3*getPopulation()))
				{
					return;
				}
			}
		}
	}
 
    
	// -------------------- BBAI Notes -------------------------
	// Minimal attack force, both land and sea
    if (bDanger) 
    {
		int iAttackNeeded = 4;
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
		iAttackNeeded += std::max(0, AI_neededDefenders() - (iPlotCityDefenderCount + iPlotOtherCityAICount));
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

		if( kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK) <  iAttackNeeded)
		{
    		if (AI_chooseUnit(UNITAI_ATTACK))
    		{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses danger minimal attack", getName().GetCString());
    			return;
    		}
		}
    }
    
    if (bMaybeWaterArea)
	{
		if( !(bLandWar && iWarSuccessRatio < -30) && !bDanger && !bFinancialTrouble )
		{
			if (kPlayer.AI_getNumTrainAIUnits(UNITAI_ATTACK_SEA) + kPlayer.AI_getNumTrainAIUnits(UNITAI_PIRATE_SEA) + kPlayer.AI_getNumTrainAIUnits(UNITAI_RESERVE_SEA) < std::min(3,kPlayer.getNumCities()))
			{
				if ((bMaybeWaterArea && bWaterDanger)
					|| (pWaterArea != NULL && bPrimaryArea && kPlayer.AI_countNumAreaHostileUnits(pWaterArea, true, false, false, false) > 0))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses minimal naval", getName().GetCString());

					if (AI_chooseUnit(UNITAI_ATTACK_SEA))
					{
						return;
					}
					if (AI_chooseUnit(UNITAI_PIRATE_SEA))
					{
						return;
					}
					if (AI_chooseUnit(UNITAI_RESERVE_SEA))
					{
						return;
					}
				}
			}
		
			if (NULL != pWaterArea)
			{
				int iOdds = -1;
				if (iAreaBestFoundValue == 0 || iWaterAreaBestFoundValue > iAreaBestFoundValue)
				{
					iOdds = 100;
				}
				else if (iWaterPercent > 60)
				{
					iOdds = 13;
				}

				if( iOdds >= 0 )
				{
					if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) == 0)
					{
						if (AI_chooseUnit(UNITAI_EXPLORE_SEA, iOdds))
						{
							if( gCityLogLevel >= 2 ) logBBAI("      City %S uses early sea explore", getName().GetCString());
							return;
						}
					}

					// BBAI TODO: Really only want to do this if no good area city sites ... 13% chance on water heavy maps
					// of slow start, little benefit
					if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) == 0)
					{
						if (AI_chooseUnit(UNITAI_SETTLER_SEA, iOdds))
						{
							if( gCityLogLevel >= 2 ) logBBAI("      City %S uses early settler sea", getName().GetCString());
							return;
						}
					}
				}
			}
		}
	}
    
	// -------------------- BBAI Notes -------------------------
	// Top normal priorities
	
	if (!bPrimaryArea && !bLandWar)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, 60, 10 + 2*iWarTroubleThreshold, 50))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose BUILDINGFOCUS_FOOD 1", getName().GetCString());
			return;
		}
	}
	
	if (!bDanger && ((kPlayer.getCurrentEra() > (GC.getGame().getStartEra() + iProductionRank / 2))) || (kPlayer.getCurrentEra() > (GC.getNumEraInfos() / 2)))
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 20 - iWarTroubleThreshold, 15, ((bLandWar || bAssault) ? 25 : -1)))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose BUILDINGFOCUS_PRODUCTION 1", getName().GetCString());
			return;	
		}

		if( !(bDefenseWar && iWarSuccessRatio < -30) )
		{
			if ((iExistingWorkers < ((iNeededWorkers + 1) / 2)))
			{
				if( getPopulation() > 3 || (iProductionRank < (kPlayer.getNumCities() + 1) / 2) )
				{
					if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 3", getName().GetCString());
						return;
					}
					bChooseWorker = true;
				}
			}
		}
	}
	
	bool bCrushStrategy = kPlayer.AI_isDoStrategy(AI_STRATEGY_CRUSH);
	int iNeededFloatingDefenders = (isBarbarian() || bCrushStrategy) ?  0 : kPlayer.AI_getTotalFloatingDefendersNeeded(pArea);
 	int iTotalFloatingDefenders = (isBarbarian() ? 0 : kPlayer.AI_getTotalFloatingDefenders(pArea));
	
	UnitTypeWeightArray floatingDefenderTypes;
	floatingDefenderTypes.push_back(std::make_pair(UNITAI_CITY_DEFENSE, 125));
	floatingDefenderTypes.push_back(std::make_pair(UNITAI_CITY_COUNTER, 100));
	//floatingDefenderTypes.push_back(std::make_pair(UNITAI_CITY_SPECIAL, 0));
	floatingDefenderTypes.push_back(std::make_pair(UNITAI_RESERVE, 100));
	floatingDefenderTypes.push_back(std::make_pair(UNITAI_COLLATERAL, 100));
	
	if (iTotalFloatingDefenders < ((iNeededFloatingDefenders + 1) / (bGetBetterUnits ? 3 : 2)))
	{
		if (AI_chooseLeastRepresentedUnit(floatingDefenderTypes))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose floating defender 1", getName().GetCString());
			return;
		}
	}

	// If losing badly in war, need to build up defenses and counter attack force
	if( bLandWar && (iWarSuccessRatio < -30 || iEnemyPowerPerc > 150) )
	{
		UnitTypeWeightArray defensiveTypes;
		defensiveTypes.push_back(std::make_pair(UNITAI_COUNTER, 100));
		defensiveTypes.push_back(std::make_pair(UNITAI_ATTACK, 100));
		defensiveTypes.push_back(std::make_pair(UNITAI_RESERVE, 60));
		defensiveTypes.push_back(std::make_pair(UNITAI_COLLATERAL, 60));
		if ( bDanger || (iTotalFloatingDefenders < (5*iNeededFloatingDefenders)/(bGetBetterUnits ? 6 : 4)))
		{
			defensiveTypes.push_back(std::make_pair(UNITAI_CITY_DEFENSE, 200));
			defensiveTypes.push_back(std::make_pair(UNITAI_CITY_COUNTER, 50));
		}

		int iOdds = iBuildUnitProb;
		if( iWarSuccessRatio < -50 )
		{
			iOdds += abs(iWarSuccessRatio/3);
		}
		if( bDanger )
		{
			iOdds += 10;
		}

		if (AI_chooseLeastRepresentedUnit(defensiveTypes, iOdds))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose losing extra defense with odds %d", getName().GetCString(), iOdds);
			return;
		}
	}

	if( !(bDefenseWar && iWarSuccessRatio < -50) )
	{
		if (!(iExistingWorkers == 0))
		{
			if (!bDanger && (iExistingWorkers < ((iNeededWorkers + 1) / 2)))
			{
				if( getPopulation() > 3 || (iProductionRank < (kPlayer.getNumCities() + 1) / 2) )
				{
					if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 4", getName().GetCString());
						return;
					}
					bChooseWorker = true;
				}
			}
		}
	}
	
	//do a check for one tile island type thing?
    //this can be overridden by "wait and grow more"
    if (!bDanger && (iExistingWorkers == 0) && (isCapital() || (iNeededWorkers > 0) || (iNeededSeaWorkers > iExistingSeaWorkers)))
    {
		if( !(bDefenseWar && iWarSuccessRatio < -30) && !(kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE)) )
		{
			if ((AI_countNumBonuses(NO_BONUS, /*bIncludeOurs*/ true, /*bIncludeNeutral*/ true, -1, /*bLand*/ true, /*bWater*/ false) > 0) || 
				(isCapital() && (getPopulation() > 3) && iNumCitiesInArea > 1))
    		{
				if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 5", getName().GetCString());
					return;
				}
				bChooseWorker = true;
    		}

			if (iNeededSeaWorkers > iExistingSeaWorkers)
			{
				if (AI_chooseUnit(UNITAI_WORKER_SEA))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker sea 2", getName().GetCString());
					return;
				}
			}
		}
    }

	if( !(bDefenseWar && iWarSuccessRatio < -30) )
	{
		if (!bWaterDanger && iNeededSeaWorkers > iExistingSeaWorkers)
		{
			if (AI_chooseUnit(UNITAI_WORKER_SEA))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker sea 3", getName().GetCString());
				return;
			}
		}
	}

	if	(!bLandWar && !bAssault && (iTargetCulturePerTurn > getCommerceRate(COMMERCE_CULTURE)))
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, bAggressiveAI ? 10 : 20, 0, bAggressiveAI ? 33 : 50))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses minimal culture rate", getName().GetCString());
			return;
		}
	}
	
	int iMinFoundValue = kPlayer.AI_getMinFoundValue();
	if (bDanger)
	{
		iMinFoundValue *= 3;
		iMinFoundValue /= 2;
	}

	// BBAI TODO: Check that this works to produce early rushes on tight maps
	if (!bGetBetterUnits && (bIsCapitalArea) && (iAreaBestFoundValue < (iMinFoundValue * 2)))
	{
		//Building city hunting stack.

		if ((getDomainFreeExperience(DOMAIN_LAND) == 0) && (getYieldRate(YIELD_PRODUCTION) > 4))
		{
    		if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, (kPlayer.getCurrentEra() > 1) ? 0 : 7, 33))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses special BUILDINGFOCUS_EXPERIENCE 1a", getName().GetCString());
				return;
			}
		}

		int iStartAttackStackRand = 0;
		if (pArea->getCitiesPerPlayer(BARBARIAN_PLAYER) > 0)
		{
			iStartAttackStackRand += 15;
		}
		if ((pArea->getNumCities() - iNumCitiesInArea) > 0)
		{
			iStartAttackStackRand += iBuildUnitProb / 2;
		}

		if( iStartAttackStackRand > 0 )
		{
			int iAttackCityCount = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK_CITY);
			int iAttackCount = iAttackCityCount + kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK);

			if( (iAttackCount) == 0 )
			{
				if( !bFinancialTrouble )
				{
					if (AI_chooseUnit(UNITAI_ATTACK, iStartAttackStackRand))
					{
						return;
					}
				}
			}
			else
			{
				if( (iAttackCount > 1) && (iAttackCityCount == 0) )
				{
					if (AI_chooseUnit(UNITAI_ATTACK_CITY))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose start city attack stack", getName().GetCString());
						return;
					}
				}
				else if (iAttackCount < (3 + iBuildUnitProb / 10))
				{
					if (AI_chooseUnit(UNITAI_ATTACK))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose add to city attack stack", getName().GetCString());
						return;
					}
				}
			}
		}
	}

	//opportunistic wonder build (1)
	if (!bDanger && (!hasActiveWorldWonder()) && (kPlayer.getNumCities() <= 3))
	{
		// For small civ at war, don't build wonders unless winning
		if( !bLandWar || (iWarSuccessRatio > 30) )
		{
			int iWonderTime = GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
			iWonderTime /= 5;
			iWonderTime += 7;
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, iWonderTime))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses oppurtunistic wonder build 1", getName().GetCString());
				return;
			}
		}
	}
	
	if (!bDanger && !bIsCapitalArea && area()->getCitiesPerPlayer(getOwnerINLINE()) > iNumCapitalAreaCities)
	{
		// BBAI TODO:  This check should be done by player, not by city and optimize placement
		// If losing badly in war, don't build big things
		if( !bLandWar || (iWarSuccessRatio > -30) )
		{
			if( kPlayer.getCapitalCity() == NULL || area()->getPopulationPerPlayer(getOwnerINLINE()) > kPlayer.getCapitalCity()->area()->getPopulationPerPlayer(getOwnerINLINE()) )
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_CAPITAL, 15))
				{
					return;
				}
			}
		}
	}
	
	if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, isCapital() ? 5 : 30, 30))
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose BUILDINGFOCUS_FOOD 2", getName().GetCString());
		return;
	}

	int iSpreadUnitThreshold = 1000;

	if( bLandWar )
	{
		iSpreadUnitThreshold += 800 - 10*iWarSuccessRatio;
	}
	iSpreadUnitThreshold += 300*plot()->plotCount(PUF_isUnitAIType, UNITAI_MISSIONARY, -1, getOwnerINLINE());

	UnitTypes eBestSpreadUnit = NO_UNIT;
	int iBestSpreadUnitValue = -1;
	
	if( !bDanger && !(kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE)) )
	{
		int iSpreadUnitRoll = (100 - iBuildUnitProb) / 3;
		iSpreadUnitRoll += bLandWar ? 0 : 10;
		
		if (AI_bestSpreadUnit(true, true, iSpreadUnitRoll, &eBestSpreadUnit, &iBestSpreadUnitValue))
		{
			if (iBestSpreadUnitValue > iSpreadUnitThreshold)
			{
				if (AI_chooseUnit(eBestSpreadUnit, UNITAI_MISSIONARY))
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose missionary 1", getName().GetCString());
					return;
				}
				FAssertMsg(false, "AI_bestSpreadUnit should provide a valid unit when it returns true");
			}
		}
	}
	
	if( !(bLandWar && iWarSuccessRatio < 30) )
	{
		if (!bDanger && (iProductionRank <= ((kPlayer.getNumCities() / 5) + 1)))
		{
			// BBAI TODO: Temporary for testing
			//if( getOwnerINLINE()%2 == 1 )
			//{
				if (AI_chooseProject())
				{
					if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose project 1", getName().GetCString());
					return;
				}
			//}
		}
	}
	
	//minimal defense.
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
	if (iPlotCityDefenderCount + iPlotOtherCityAICount < (AI_minDefenders() + iPlotSettlerCount))
	{
		if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose min defender", getName().GetCString());
			return;
		}

		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose min defender (city counter ai)", getName().GetCString());
			return;
		}

		if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose min defender (city special ai)", getName().GetCString());
			return;
		}

		if (AI_chooseUnit(UNITAI_RESERVE))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose min defender (reserve ai)", getName().GetCString());
			return;
		}

		if (AI_chooseUnit(UNITAI_ATTACK))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose min defender (attack ai)", getName().GetCString());
			return;
		}

//Fuyu bIgnoreNotUnitAIs
		if (iNumCitiesInArea <= 3)
		{
			if (AI_chooseUnit(UNITAI_CITY_DEFENSE, -1, true))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose min defender (IgnoreNotUnitAIs)", getName().GetCString());
				return;
			}
		}
	}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

	if( !(bDefenseWar && iWarSuccessRatio < -50) )
	{
		if ((iAreaBestFoundValue > iMinFoundValue) || (iWaterAreaBestFoundValue > iMinFoundValue))
		{
			// BBAI TODO: Needs logic to check for early settler builds, settler builds in small cities, whether settler sea exists for water area sites?
			if (pWaterArea != NULL)
			{
				int iTotalCities = kPlayer.getNumCities();
				int iSettlerSeaNeeded = std::min(iNumWaterAreaCitySites, ((iTotalCities + 4) / 8) + 1);
				if (kPlayer.getCapitalCity() != NULL)
				{
					int iOverSeasColonies = iTotalCities - kPlayer.getCapitalCity()->area()->getCitiesPerPlayer(getOwnerINLINE());
					int iLoop = 2;
					int iExtras = 0;
					while (iOverSeasColonies >= iLoop)
					{
						iExtras++;
						iLoop += iLoop + 2;
					}
					iSettlerSeaNeeded += std::min(kPlayer.AI_totalUnitAIs(UNITAI_WORKER) / 4, iExtras);
				}
				if (bAssault)
				{
					iSettlerSeaNeeded = std::min(1, iSettlerSeaNeeded);
				}
				
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) < iSettlerSeaNeeded)
				{
					if (AI_chooseUnit(UNITAI_SETTLER_SEA))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses main settler sea", getName().GetCString());
/********************************************************************************/
/* 	Build more workers #1.1										Fuyu		    */
/********************************************************************************/
						/* financial trouble: 2/3; */
						if (!bDanger && bFinancialTrouble && (iExistingWorkers < ((2*iNeededWorkers) + 2)/3))
						{
							if ((AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
							{
								if( getPopulation() > 1 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
								{
									if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
									{
										if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 6a", getName().GetCString());
										return;
									}
									bChooseWorker = true;
								}
							}
						}
/********************************************************************************/
/* 	Build more workers #1.1										END			    */
/********************************************************************************/
						return;
					}
				}
			}
			
			if (iPlotSettlerCount == 0)
			{
				if ((iNumSettlers < iMaxSettlers))
				{
					if (AI_chooseUnit(UNITAI_SETTLE, bLandWar ? 50 : -1))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1", getName().GetCString());
/********************************************************************************/
/* 	Build more workers #1.2										Fuyu		    */
/********************************************************************************/
						/* financial trouble: 2/3; */
						if (!bDanger && bFinancialTrouble && (iExistingWorkers < ((2*iNeededWorkers) + 2)/3))
						{
							if ((AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
							{
								if( getPopulation() > 1 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
								{
									if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
									{
										if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 6a", getName().GetCString());
										return;
									}
									bChooseWorker = true;
								}
							}
						}
/********************************************************************************/
/* 	Build more workers #1.2										END			    */
/********************************************************************************/
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
						if ((kPlayer.getNumMilitaryUnits() <= (kPlayer.getNumCities() + iNumSettlers + 1))
							//Fuyu: in the beginning, don't count on other cities to build the escorts
							|| (kPlayer.getNumCities() <= 7 && iNumCitiesInArea <= 3 && (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) <= 1)))
						{
							if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
							{
								if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1 extra quick defense", getName().GetCString());
								return;
							}
//Fuyu bIgnoreNotUnitAIs
							if (iNumCitiesInArea <= 3)
							{
								if (AI_chooseUnit(UNITAI_CITY_DEFENSE, -1, true))
								{
									if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1 extra quick defense (IgnoreNotUnitAIs)", getName().GetCString());
									return;
								}
							}

							if (AI_chooseUnit(UNITAI_CITY_COUNTER))
							{
								if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1 extra quick defense (city counter ai)", getName().GetCString());
								return;
							}
							if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
							{
								if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1 extra quick defense (city special ai)", getName().GetCString());
								return;
							}

							if (AI_chooseUnit(UNITAI_RESERVE))
							{
								if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1 extra quick defense (reserve ai)", getName().GetCString());
								return;
							}
						}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/
						//if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 1 NOW with %d defenders already in place", getName().GetCString(), (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType)));
						return;
					}
				}
			}
		}
	}

/********************************************************************************/
/* 	Build more workers #2										Fuyu		    */
/********************************************************************************/
	if( !(bLandWar && iWarSuccessRatio < 0) && !bDanger )
	{
		/* financial trouble: ---; will grow above happy cap: 2/3; both: 3/4; else 4/7 */
		if ( (iExistingWorkers < ((4*iNeededWorkers) + 6)/7)
			/* || (bFinancialTrouble && (iExistingWorkers < (((2*iNeededWorkers) + 1)/3))) */
			|| (((iExistingWorkers < ((2*iNeededWorkers) + 2)/3) || (bFinancialTrouble && (iExistingWorkers < (((3*iNeededWorkers) + 3)/4))))
				&& (((happyLevel() - unhappyLevel()) <= 0) && (foodDifference(false) > 0 || (foodDifference(false) == 0 && happyLevel() - unhappyLevel() < 0)))) )
		{
			if ((AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
			{
				if( getPopulation() > 2 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
				{
					if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
					{
						if (!isFoodProduction() && ((happyLevel() - unhappyLevel()) <= 0)
							&& getProductionUnit() != NO_UNIT && getUnitProduction(getProductionUnit()) == 0)
						{
							popOrder(0);
							bChooseWorker = true;

							//Already set by chooseUnit but I'm not taking any chances
							if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
							{
								setInfoDirty(true);

								if (isCitySelected())
								{
									gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true );
									gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
									gDLL->getInterfaceIFace()->setDirty(CityScreen_DIRTY_BIT, true);
									gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
								}
							}
						}
						else
						{
							if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 6b", getName().GetCString());
							return;
						}
					}
					bChooseWorker = !bChooseWorker;
				}
			}
		}
	}
/********************************************************************************/
/* 	Build more workers #2										END			   */
/********************************************************************************/


	//this is needed to build the cathedrals quickly
	//also very good for giving cultural cities first dibs on wonders
    if (bImportantCity && (iCultureRateRank <= iCulturalVictoryNumCultureCities))
    {
        if (iCultureRateRank == iCulturalVictoryNumCultureCities)
        {
            if (AI_chooseBuilding(BUILDINGFOCUS_BIGCULTURE | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_WONDEROK, 40))
            {
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses cultural victory 1", getName().GetCString());
                return;
            }
		}
        else if (GC.getGameINLINE().getSorenRandNum(((iCultureRateRank == 1) ? 4 : 1) + iCulturalVictoryNumCultureCities * 2 + (bLandWar ? 5 : 0), "AI Build up Culture") < iCultureRateRank)
        {
            if (AI_chooseBuilding(BUILDINGFOCUS_BIGCULTURE | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_WONDEROK, (bLandWar ? 20 : 40)))
            {
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses cultural victory 2", getName().GetCString());
                return;
            }
        }
    }

	// don't build frivolous things if this is an important city unless we at war
    if (!bImportantCity || bLandWar || bAssault)
    {
        if (bPrimaryArea)
        {
            if (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK) == 0)
            {
                if (AI_chooseUnit(UNITAI_ATTACK))
                {
                    return;
                }
            }
        }

        if (!bLandWar && !bDanger && !bFinancialTrouble)
        {
			if (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_EXPLORE) < (kPlayer.AI_neededExplorers(pArea)))
			{
				if (AI_chooseUnit(UNITAI_EXPLORE))
				{
					return;
				}
			}
        }

		if( bDefenseWar || (bLandWar && (iWarSuccessRatio < -30)) )
		{
			UnitTypeWeightArray panicDefenderTypes;
			panicDefenderTypes.push_back(std::make_pair(UNITAI_RESERVE, 100));
			panicDefenderTypes.push_back(std::make_pair(UNITAI_COUNTER, 100));
			panicDefenderTypes.push_back(std::make_pair(UNITAI_COLLATERAL, 100));
			panicDefenderTypes.push_back(std::make_pair(UNITAI_ATTACK, 100));

        	if (AI_chooseLeastRepresentedUnit(panicDefenderTypes, (bGetBetterUnits ? 40 : 60) - iWarSuccessRatio/3))
        	{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose panic defender", getName().GetCString());
        		return;
        	}
        }
    }
        
	if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, 60, 10, (bLandWar ? 30 : -1)))
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose BUILDINGFOCUS_FOOD 3", getName().GetCString());
		return;
	}
	
	//oppurunistic wonder build
	if (!bDanger && (!hasActiveWorldWonder() || (kPlayer.getNumCities() > 3)))
	{
		// For civ at war, don't build wonders if losing
		if( !bLandWar || (iWarSuccessRatio > -30) )
		{	
			int iWonderTime = GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
			iWonderTime /= 5;
			iWonderTime += 8;
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, iWonderTime))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses oppurtunistic wonder build 2", getName().GetCString());
				return;
			}
		}
	}
	
	if( !(bLandWar && iWarSuccessRatio < -30) && !bDanger )
	{
		if (iExistingWorkers < iNeededWorkers )
		{
			if ((AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
			{
				if( getPopulation() > 1 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
				{
					if (!bChooseWorker && AI_chooseUnit(UNITAI_WORKER))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose worker 6", getName().GetCString());
						return;
					}
					bChooseWorker = true;
				}
			}
		}
	}
    
	//essential economic builds
	if (AI_chooseBuilding(iEconomyFlags, 10, 25 + iWarTroubleThreshold, (bLandWar ? 40 : -1)))
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose iEconomyFlags 1", getName().GetCString());
		return;
	}

	if( !bDanger )
	{
		if (iBestSpreadUnitValue > ((iSpreadUnitThreshold * (bLandWar ? 80 : 60)) / 100))
		{
			if (AI_chooseUnit(eBestSpreadUnit, UNITAI_MISSIONARY))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose missionary 2", getName().GetCString());
				return;
			}
			FAssertMsg(false, "AI_bestSpreadUnit should provide a valid unit when it returns true");
		}
	}

	if ((getDomainFreeExperience(DOMAIN_LAND) == 0) && (getYieldRate(YIELD_PRODUCTION) > 4))
	{
    	if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, (kPlayer.getCurrentEra() > 1) ? 0 : 7, 33))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses special BUILDINGFOCUS_EXPERIENCE 1", getName().GetCString());
			return;
		}
	}
	
	int iMaxUnitSpending = (bAggressiveAI ? 6 : 3) + iBuildUnitProb / 3;

	if( kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST4) )
	{
		iMaxUnitSpending += 7;
	}
	else if( kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3) || kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3) )
	{
		iMaxUnitSpending += 3;
	}
	else if( kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST1) )
	{
		iMaxUnitSpending += 1;
	}

    if (bAlwaysPeace)
	{
		iMaxUnitSpending = -10;
	}
    else if (kPlayer.AI_isDoStrategy(AI_STRATEGY_FINAL_WAR))
    {
    	iMaxUnitSpending = 5 + iMaxUnitSpending + (100 - iMaxUnitSpending) / 2;
    }
    else
    {
    	iMaxUnitSpending += bDefenseWar ? 4 : 0;
    	switch (pArea->getAreaAIType(getTeam()))
    	{
			case AREAAI_OFFENSIVE:
				iMaxUnitSpending += 5;
				break;
			
			case AREAAI_DEFENSIVE:
				iMaxUnitSpending += 10;
				break;
			
			case AREAAI_MASSING:
				iMaxUnitSpending += 25;
				break;
			
			case AREAAI_ASSAULT:
				iMaxUnitSpending += 8;
				break;
				
			case AREAAI_ASSAULT_MASSING:
				iMaxUnitSpending += 16;
				break;
			
			case AREAAI_ASSAULT_ASSIST:
				iMaxUnitSpending += 6;
				break;
			
			case AREAAI_NEUTRAL:
				break;
			default:
				FAssert(false);
		}
	}

	int iCarriers = kPlayer.AI_totalUnitAIs(UNITAI_CARRIER_SEA);
	
	// Revamped logic for production for invasions
    if (iUnitCostPercentage < (iMaxUnitSpending + 10))
	{
		bool bBuildAssault = bAssault;
		CvArea* pAssaultWaterArea = NULL;
		if (NULL != pWaterArea)
		{
			// Coastal city extra logic

			pAssaultWaterArea = pWaterArea;

			// If on offensive and can't reach enemy cities from here, act like using AREAAI_ASSAULT
			if( (pAssaultWaterArea != NULL) && !bBuildAssault )
			{
				if( (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0) )
				{
					if( (pArea->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE) )
					{
						// BBAI TODO: faster to switch to checking path for some selection group?
						if( !(plot()->isHasPathToEnemyCity(getTeam())) )
						{
							bBuildAssault = true;
						}
					}
				}
			}
		}

		if( bBuildAssault )
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build assault", getName().GetCString());

			UnitTypes eBestAssaultUnit = NO_UNIT; 
			if (NULL != pAssaultWaterArea)
			{
				kPlayer.AI_bestCityUnitAIValue(UNITAI_ASSAULT_SEA, this, &eBestAssaultUnit);
			}
			else
			{
				kPlayer.AI_bestCityUnitAIValue(UNITAI_ASSAULT_SEA, NULL, &eBestAssaultUnit);
			}
			
			int iBestSeaAssaultCapacity = 0;
			if (eBestAssaultUnit != NO_UNIT)
			{
				iBestSeaAssaultCapacity = GC.getUnitInfo(eBestAssaultUnit).getCargoSpace();
			}

			int iAreaAttackCityUnits = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK_CITY);
			
			int iUnitsToTransport = iAreaAttackCityUnits;
			iUnitsToTransport += kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK);
			iUnitsToTransport += kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_COUNTER)/2;

			int iLocalTransports = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ASSAULT_SEA);
			int iTransportsAtSea = 0;
			if (NULL != pAssaultWaterArea)
			{
				iTransportsAtSea = kPlayer.AI_totalAreaUnitAIs(pAssaultWaterArea, UNITAI_ASSAULT_SEA);
			}
			else
			{
				iTransportsAtSea = kPlayer.AI_totalUnitAIs(UNITAI_ASSAULT_SEA)/2;
			}

			//The way of calculating numbers is a bit fuzzy since the ships
			//can make return trips. When massing for a war it'll train enough
			//ships to move it's entire army. Once the war is underway it'll stop
			//training so many ships on the assumption that those out at sea
			//will return...

			int iTransports = iLocalTransports + (bPrimaryArea ? iTransportsAtSea : iTransportsAtSea/4);
			int iTransportCapacity = iBestSeaAssaultCapacity*(iTransports);

			if (NULL != pAssaultWaterArea)
			{
				int iEscorts = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ESCORT_SEA);
				iEscorts += kPlayer.AI_totalAreaUnitAIs(pAssaultWaterArea, UNITAI_ESCORT_SEA);

				int iTransportViability = kPlayer.AI_calculateUnitAIViability(UNITAI_ASSAULT_SEA, DOMAIN_SEA);

				int iDesiredEscorts = ((1 + 2 * iTransports) / 3);
				if( iTransportViability > 95 )
				{
					// Transports are stronger than escorts (usually Galleons and Caravels)
					iDesiredEscorts /= 3;
				}
				
				if ((iEscorts < iDesiredEscorts))
				{
					if (AI_chooseUnit(UNITAI_ESCORT_SEA, (iEscorts < iDesiredEscorts/3) ? -1 : 50))
					{
						AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 12);
						return;
					}
				}
			
				UnitTypes eBestAttackSeaUnit = NO_UNIT;  
				kPlayer.AI_bestCityUnitAIValue(UNITAI_ATTACK_SEA, this, &eBestAttackSeaUnit);
				if (eBestAttackSeaUnit != NO_UNIT)
				{
					int iDivisor = 2;
					if (GC.getUnitInfo(eBestAttackSeaUnit).getBombardRate() == 0)
					{
						iDivisor = 5;
					}

					int iAttackSea = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK_SEA);
					iAttackSea += kPlayer.AI_totalAreaUnitAIs(pAssaultWaterArea, UNITAI_ATTACK_SEA);
						
					if ((iAttackSea < ((1 + 2 * iTransports) / iDivisor)))
					{
						if (AI_chooseUnit(UNITAI_ATTACK_SEA, (iUnitCostPercentage < iMaxUnitSpending) ? 50 : 20))
						{
							AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 12);
							return;
						}
					}
				}
				
				if (iUnitsToTransport > iTransportCapacity)
				{
					if ((iUnitCostPercentage < iMaxUnitSpending) || (iUnitsToTransport > 2*iTransportCapacity))
					{
						if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
						{
							AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 8);
							return;
						}
					}
				}
			}

			if (iUnitCostPercentage < iMaxUnitSpending)
			{
				if (NULL != pAssaultWaterArea)
				{
					if (!bFinancialTrouble && iCarriers < (kPlayer.AI_totalUnitAIs(UNITAI_ASSAULT_SEA) / 4))
					{
						// Reduce chances of starting if city has low production
						if (AI_chooseUnit(UNITAI_CARRIER_SEA, (iProductionRank <= ((kPlayer.getNumCities() / 3) + 1)) ? -1 : 30))
						{
							AI_chooseBuilding(BUILDINGFOCUS_DOMAINSEA, 16);
							return;
						}
					}
				}
			}

			// Consider building more land units to invade with
			int iTrainInvaderChance = iBuildUnitProb + 10;

			iTrainInvaderChance += (bAggressiveAI ? 15 : 0);
			iTrainInvaderChance /= (bAssaultAssist ? 2 : 1);
			iTrainInvaderChance /= (bImportantCity ? 2 : 1);
			iTrainInvaderChance /= (bGetBetterUnits ? 2 : 1);

			iUnitsToTransport *= 9;
			iUnitsToTransport /= 10;

			if( (iUnitsToTransport > iTransportCapacity) && (iUnitsToTransport > (bAssaultAssist ? 2 : 4)*iBestSeaAssaultCapacity) )
			{
				// Already have enough
				iTrainInvaderChance /= 2;
			}
			else if( iUnitsToTransport < (iLocalTransports*iBestSeaAssaultCapacity) )
			{
				iTrainInvaderChance += 15;
			}

			if( getPopulation() < 4 )
			{
				// Let small cities build themselves up first
				iTrainInvaderChance /= (5 - getPopulation());
			}

			UnitTypeWeightArray invaderTypes;
			invaderTypes.push_back(std::make_pair(UNITAI_ATTACK_CITY, 100));
			invaderTypes.push_back(std::make_pair(UNITAI_COUNTER, 50));
			invaderTypes.push_back(std::make_pair(UNITAI_ATTACK, 40));
			if( kPlayer.AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ) )
			{
				invaderTypes.push_back(std::make_pair(UNITAI_PARADROP, 20));
			}

			if (AI_chooseLeastRepresentedUnit(invaderTypes, iTrainInvaderChance))
			{
				if( !bImportantCity && (iUnitsToTransport >= (iLocalTransports*iBestSeaAssaultCapacity)) )
				{
					// Have time to build barracks first
					AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20);
				}
				return;
			}

			if (iUnitCostPercentage < (iMaxUnitSpending))
			{
				int iMissileCarriers = kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_CARRIER_SEA);
			
				if (!bFinancialTrouble && iMissileCarriers > 0 && !bImportantCity)
				{
					if( (iProductionRank <= ((kPlayer.getNumCities() / 2) + 1)) )
					{
						UnitTypes eBestMissileCarrierUnit = NO_UNIT;  
						kPlayer.AI_bestCityUnitAIValue(UNITAI_MISSILE_CARRIER_SEA, NULL, &eBestMissileCarrierUnit);
						if (eBestMissileCarrierUnit != NO_UNIT)
						{
							FAssert(GC.getUnitInfo(eBestMissileCarrierUnit).getDomainCargo() == DOMAIN_AIR);
							
							int iMissileCarrierAirNeeded = iMissileCarriers * GC.getUnitInfo(eBestMissileCarrierUnit).getCargoSpace();
							
							if ((kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_AIR) < iMissileCarrierAirNeeded) || 
								(bPrimaryArea && (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_MISSILE_CARRIER_SEA) * GC.getUnitInfo(eBestMissileCarrierUnit).getCargoSpace() < kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_MISSILE_AIR))))
							{
								// Don't always build missiles, more likely if really low
								if (AI_chooseUnit(UNITAI_MISSILE_AIR, (kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_AIR) < iMissileCarrierAirNeeded/2) ? 50 : 20))
								{
									return;
								}
							}
						}
					}
				}
			}
		}
	}
	
	UnitTypeWeightArray airUnitTypes;

    int iAircraftNeed = 0;
    int iAircraftHave = 0;
    UnitTypes eBestAttackAircraft = NO_UNIT;
    UnitTypes eBestMissile = NO_UNIT;
    
	if (iUnitCostPercentage < (iMaxUnitSpending + 4) && (!bImportantCity || bDefenseWar) )
	{
		if( bLandWar || bAssault || (iFreeAirExperience > 0) || (GC.getGame().getSorenRandNum(3, "AI train air") == 0) )
		{
			int iBestAirValue = kPlayer.AI_bestCityUnitAIValue(UNITAI_ATTACK_AIR, this, &eBestAttackAircraft);
			int iBestMissileValue = kPlayer.AI_bestCityUnitAIValue(UNITAI_MISSILE_AIR, this, &eBestMissile);
			if ((iBestAirValue + iBestMissileValue) > 0)
			{
				iAircraftHave = kPlayer.AI_totalUnitAIs(UNITAI_ATTACK_AIR) + kPlayer.AI_totalUnitAIs(UNITAI_DEFENSE_AIR) + kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_AIR);
				if (NO_UNIT != eBestAttackAircraft)
				{
					iAircraftNeed = (2 + kPlayer.getNumCities() * (3 * GC.getUnitInfo(eBestAttackAircraft).getAirCombat())) / (2 * std::max(1, GC.getGame().getBestLandUnitCombat()));
					int iBestDefenseValue = kPlayer.AI_bestCityUnitAIValue(UNITAI_DEFENSE_AIR, this);
					if ((iBestDefenseValue > 0) && (iBestAirValue > iBestDefenseValue))
					{
						iAircraftNeed *= 3;
						iAircraftNeed /= 2;
					}
				}
				if (iBestMissileValue > 0)
				{
					iAircraftNeed = std::max(iAircraftNeed, 1 + kPlayer.getNumCities() / 2);
				}
				
				bool bAirBlitz = kPlayer.AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ);
				bool bLandBlitz = kPlayer.AI_isDoStrategy(AI_STRATEGY_LAND_BLITZ);
				if (bAirBlitz)
				{
					iAircraftNeed *= 3;
					iAircraftNeed /= 2;
				}
				else if (bLandBlitz)
				{
					iAircraftNeed /= 2;
					iAircraftNeed += 1;
				}
				
				airUnitTypes.push_back(std::make_pair(UNITAI_ATTACK_AIR, bAirBlitz ? 125 : 80));
				airUnitTypes.push_back(std::make_pair(UNITAI_DEFENSE_AIR, bLandBlitz ? 100 : 100));
				if (iBestMissileValue > 0)
				{
					airUnitTypes.push_back(std::make_pair(UNITAI_MISSILE_AIR, bAssault ? 60 : 40));
				}
				
				airUnitTypes.push_back(std::make_pair(UNITAI_ICBM, 20));
				
				if (iAircraftHave * 2 < iAircraftNeed)
				{
					if (AI_chooseLeastRepresentedUnit(airUnitTypes))
					{
						return;
					}
				}
				// Additional check for air defenses
				int iFightersHave = kPlayer.AI_totalUnitAIs(UNITAI_DEFENSE_AIR);

				if( 3*iFightersHave < iAircraftNeed )
				{
					if (AI_chooseUnit(UNITAI_DEFENSE_AIR))
					{
						return;
					}
				}
			}
		}
	}

	// Check for whether to produce planes to fill carriers
	if ( (bLandWar || bAssault) && iUnitCostPercentage < (iMaxUnitSpending))
	{			
		if (iCarriers > 0 && !bImportantCity)
		{
			UnitTypes eBestCarrierUnit = NO_UNIT;  
			kPlayer.AI_bestCityUnitAIValue(UNITAI_CARRIER_SEA, NULL, &eBestCarrierUnit);
			if (eBestCarrierUnit != NO_UNIT)
			{
				FAssert(GC.getUnitInfo(eBestCarrierUnit).getDomainCargo() == DOMAIN_AIR);
				
				int iCarrierAirNeeded = iCarriers * GC.getUnitInfo(eBestCarrierUnit).getCargoSpace();

				// Reduce chances if city gives no air experience
				if (kPlayer.AI_totalUnitAIs(UNITAI_CARRIER_AIR) < iCarrierAirNeeded)
				{
					if (AI_chooseUnit(UNITAI_CARRIER_AIR, (iFreeAirExperience > 0) ? -1 : 35))
					{
						return;
					}
				}
			}
		}
	}
	
	if (!bAlwaysPeace && !(bLandWar || bAssault) && (kPlayer.AI_isDoStrategy(AI_STRATEGY_OWABWNW) || (GC.getGame().getSorenRandNum(12, "AI consider Nuke") == 0)))
	{
		if( !bFinancialTrouble )
		{
			int iTotalNukes = kPlayer.AI_totalUnitAIs(UNITAI_ICBM);
			int iNukesWanted = 1 + 2 * std::min(kPlayer.getNumCities(), GC.getGame().getNumCities() - kPlayer.getNumCities());
			if ((iTotalNukes < iNukesWanted) && (GC.getGame().getSorenRandNum(100, "AI train nuke MWAHAHAH") < (90 - (80 * iTotalNukes) / iNukesWanted)))
			{
				if ((pWaterArea != NULL))
				{
					if (AI_chooseUnit(UNITAI_MISSILE_CARRIER_SEA, 50))
					{
						return;	
					}
				}

				if (AI_chooseUnit(UNITAI_ICBM))
				{
					return;
				}
			}
		}
	}   

	// Assault case now completely handled above
	if (!bAssault && (!bImportantCity || bDefenseWar) && (iUnitCostPercentage < iMaxUnitSpending))
    {
        if (!bFinancialTrouble && (bLandWar || (kPlayer.AI_isDoStrategy(AI_STRATEGY_DAGGER) && !bGetBetterUnits)))
        {
        	int iTrainInvaderChance = iBuildUnitProb + 10;

        	if (bAggressiveAI)
        	{
        		iTrainInvaderChance += 15;
        	}

			if( bGetBetterUnits )
			{
				iTrainInvaderChance /= 2;
			}
        	else if ((pArea->getAreaAIType(getTeam()) == AREAAI_MASSING) || (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT_MASSING))
        	{
        		iTrainInvaderChance = (100 - ((100 - iTrainInvaderChance) / (bCrushStrategy ? 6 : 3)));
        	}        	

			if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20, 0, bDefenseWar ? 10 : 30))
			{
				return;
			}

			UnitTypeWeightArray invaderTypes;
			invaderTypes.push_back(std::make_pair(UNITAI_ATTACK_CITY, 100));
			invaderTypes.push_back(std::make_pair(UNITAI_COUNTER, 50));
			invaderTypes.push_back(std::make_pair(UNITAI_ATTACK, 40));
			invaderTypes.push_back(std::make_pair(UNITAI_PARADROP, (kPlayer.AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ) ? 30 : 20) / (bAssault ? 2 : 1)));
			if (!bAssault)
			{
				if (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_PILLAGE) <= ((iNumCitiesInArea + 1) / 2))
				{
					invaderTypes.push_back(std::make_pair(UNITAI_PILLAGE, 30));
				}
			}

			if (AI_chooseLeastRepresentedUnit(invaderTypes, iTrainInvaderChance))
			{
				return;
			}
        }
	}

	if ((pWaterArea != NULL) && !bDefenseWar && !bAssault)
	{
		if( !bFinancialTrouble )
		{
			// Force civs with foreign colonies to build a few assault transports to defend the colonies
			if( kPlayer.AI_totalUnitAIs(UNITAI_ASSAULT_SEA) < (kPlayer.getNumCities() - iNumCapitalAreaCities)/3 )
			{
				if (AI_chooseUnit(UNITAI_ASSAULT_SEA))
				{
					return;
				}
			}

			if (kPlayer.AI_calculateUnitAIViability(UNITAI_SETTLER_SEA, DOMAIN_SEA) < 61)
			{
				// Force civs to build escorts for settler_sea units
				if( kPlayer.AI_totalUnitAIs(UNITAI_SETTLER_SEA) > kPlayer.AI_getNumAIUnits(UNITAI_RESERVE_SEA) )
				{
					if (AI_chooseUnit(UNITAI_RESERVE_SEA))
					{
						return;
					}
				}
			}
		}
	}
	
	//Arr.  Don't build pirates in financial trouble, as they'll be disbanded with high probability
	if ((pWaterArea != NULL) && !bLandWar && !bAssault && !bFinancialTrouble)
	{
		int iPirateCount = kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_PIRATE_SEA);
		int iNeededPirates = (1 + (pWaterArea->getNumTiles() / std::max(1, 200 - iBuildUnitProb)));
		iNeededPirates *= (20 + iWaterPercent);
		iNeededPirates /= 100;
		
		if (kPlayer.isNoForeignTrade())
		{
			iNeededPirates *= 3;
			iNeededPirates /= 2;
		}
		if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_PIRATE_SEA) < iNeededPirates)
		{
			if (kPlayer.AI_calculateUnitAIViability(UNITAI_PIRATE_SEA, DOMAIN_SEA) > 49)
			{
				if (AI_chooseUnit(UNITAI_PIRATE_SEA, iWaterPercent / (1 + iPirateCount)))
				{
					return;
				}
			}
		}
	}
	
	if (!bLandWar && !bFinancialTrouble)
	{
		if ((pWaterArea != NULL) && (iWaterPercent > 40))
		{
			if (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_SPY) > 0)
			{
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SPY_SEA) == 0)
				{
					if (AI_chooseUnit(UNITAI_SPY_SEA))
					{
						return;
					}
				}
			}
		}
	}
	
	if (iBestSpreadUnitValue > ((iSpreadUnitThreshold * 40) / 100))
	{
		if (AI_chooseUnit(eBestSpreadUnit, UNITAI_MISSIONARY))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose missionary 3", getName().GetCString());
			return;
		}
		FAssertMsg(false, "AI_bestSpreadUnit should provide a valid unit when it returns true");
	}
		
	if (iTotalFloatingDefenders < iNeededFloatingDefenders && (!bFinancialTrouble || bLandWar))
	{
		if (AI_chooseLeastRepresentedUnit(floatingDefenderTypes, 50))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose floating defender 2", getName().GetCString());
			return;
		}
	}
	
	int iNumSpies = (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_SPY));
	int iNeededSpies = iNumCitiesInArea / 3;
	iNeededSpies += isCapital() ? 1 : 0;
	if (iNumSpies < iNeededSpies)
	{
		if (AI_chooseUnit(UNITAI_SPY, 5 + 50 / (1 + iNumSpies)))
		{
			return;
		}
	}
	
	if (bLandWar && !bDanger)
	{
		if (iNumSettlers < iMaxSettlers)
		{
			if (!bFinancialTrouble)
			{
				if (iAreaBestFoundValue > iMinFoundValue)
				{
					if (AI_chooseUnit(UNITAI_SETTLE))
					{
						if( gCityLogLevel >= 2 ) logBBAI("      City %S uses build settler 2", getName().GetCString());
						return;
					}
				}
			}
		}
	}
	
	if ((iProductionRank <= ((kPlayer.getNumCities() > 8) ? 3 : 2))
		&& (getPopulation() > 3))
	{
		int iWonderRand = 8 + GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
		
		// increase chance of going for an early wonder
		if (GC.getGameINLINE().getElapsedGameTurns() < (100 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent() / 100) && iNumCitiesInArea > 1)
		{
			iWonderRand *= 35;
			iWonderRand /= 100;
		}
		else if (iNumCitiesInArea >= 3)
		{
			iWonderRand *= 30;
			iWonderRand /= 100;
		}
		else
		{
			iWonderRand *= 25;
			iWonderRand /= 100;
		}
		
		if (bAggressiveAI)
		{
			iWonderRand *= 2;
			iWonderRand /= 3;
		}
		
		int iWonderRoll = GC.getGameINLINE().getSorenRandNum(100, "Wonder Build Rand");
		
		if (iProductionRank == 1)
		{
			iWonderRoll /= 2;
		}
		
		if (iWonderRoll < iWonderRand)
		{
			int iWonderMaxTurns = 20 + ((iWonderRand - iWonderRoll) * 2);
			if (bLandWar)
			{
				iWonderMaxTurns /= 2;
			}
			
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, iWonderMaxTurns))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses oppurtunistic wonder build 3", getName().GetCString());
				return;
			}
		}
	}
	
	if (iUnitCostPercentage < iMaxUnitSpending + 4 && !bFinancialTrouble)
	{
		if ((iAircraftHave * 2 >= iAircraftNeed) && (iAircraftHave < iAircraftNeed))
		{
			int iOdds = 33;

			if( iFreeAirExperience > 0 || (iProductionRank <= (1 + kPlayer.getNumCities()/2)) )
			{
				iOdds = -1;
			}

			if (AI_chooseLeastRepresentedUnit(airUnitTypes, iOdds))
			{
				return;
			}
		}
	}

	if (!bLandWar)
	{		
		if ((iCulturePressure > 90) || kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 20))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses cultural pressure/cultural victory 3", getName().GetCString());
				return;
			}
		}

		if (pWaterArea != NULL && bFinancialTrouble)
		{
			if (kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_MISSIONARY) > 0)
			{
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_MISSIONARY_SEA) == 0)
				{
					if (AI_chooseUnit(UNITAI_MISSIONARY_SEA))
					{
						return;
					}
				}
			}
		}
	}


	if (getCommerceRateTimes100(COMMERCE_CULTURE) == 0)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30))
		{
			return;
		}
	}

	if (!bAlwaysPeace )
	{
	    if (!bDanger)
	    {
			if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20, 0, 3*getPopulation()))
            {
                return;
            }
	    }

		if (AI_chooseBuilding(BUILDINGFOCUS_DEFENSE, 20, 0, bDanger ? -1 : 3*getPopulation()))
		{
			return;
		}
		
		if (bDanger)
	    {
            if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 20, 0, 2*getPopulation()))
            {
                return;
            }
	    }
	}

	if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 20, 4))
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose BUILDINGFOCUS_PRODUCTION 2", getName().GetCString());
		return;
	}
	
	//20 means 5g or ~2 happiness...
	if (AI_chooseBuilding(iEconomyFlags, 15, 20))
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose iEconomyFlags 2", getName().GetCString());
		return;
	}


	if (!bLandWar)
	{
		if (AI_chooseBuilding(iEconomyFlags, 40, 8))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose iEconomyFlags 3", getName().GetCString());
			return;
		}

		if (iCulturePressure > 50)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 60))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose cultural pressure 2", getName().GetCString());
				return;
			}
		}

		if (pWaterArea != NULL)
		{
			if (bPrimaryArea)
			{
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) < std::min(1, kPlayer.AI_neededExplorers(pWaterArea)))
				{
					if (AI_chooseUnit(UNITAI_EXPLORE_SEA))
					{
						return;
					}
				}
			}
		}

		if (getBaseYieldRate(YIELD_PRODUCTION) >= 8)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 80))
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose BUILDINGFOCUS_PRODUCTION 3", getName().GetCString());
				return;
			}
		}
	}

	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_COUNTER, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			return;
		}
	}

	// we do a similar check lower, in the landwar case
	if (!bLandWar && bFinancialTrouble)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_GOLD))
		{
			if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose financial trouble gold", getName().GetCString());
			return;
		}
	}

	bChooseUnit = false;
	if (iUnitCostPercentage < iMaxUnitSpending + 5)
	{
		if ((bLandWar) ||
			  ((kPlayer.getNumCities() <= 3) && (GC.getGameINLINE().getElapsedGameTurns() < 60)) ||
			  (GC.getGameINLINE().getSorenRandNum(100, "AI Build Unit Production") < AI_buildUnitProb()) ||
				(isHuman() && (getGameTurnFounded() == GC.getGameINLINE().getGameTurn())))
		{
			if (AI_chooseUnit())
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose unit by probability", getName().GetCString());
				return;
			}

			bChooseUnit = true;
		}
	}

	// BBAI TODO: Temporary for testing
	//if( getOwnerINLINE()%2 == 1 )
	//{
		// Only cities with reasonable production
		if ((iProductionRank <= ((kPlayer.getNumCities() > 8) ? 3 : 2))
		&& (getPopulation() > 3))
		{
			if (AI_chooseProject())
			{
				if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose project 2", getName().GetCString());
				return;
			}
		}
	//}

	if (AI_chooseBuilding())
	{
		if( gCityLogLevel >= 2 ) logBBAI("      City %S uses choose building by probability", getName().GetCString());
		return;
	}
	
	if (!bChooseUnit && !bFinancialTrouble && kPlayer.AI_isDoStrategy(AI_STRATEGY_FINAL_WAR))
	{
		if (AI_chooseUnit())
		{
			return;
		}
	}

	if (AI_chooseProcess())
	{
		return;
	}
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


UnitTypes CvCityAI::AI_bestUnit(bool bAsync, AdvisorTypes eIgnoreAdvisor, UnitAITypes* peBestUnitAI)
{
	CvArea* pWaterArea;
	int aiUnitAIVal[NUM_UNITAI_TYPES];
	UnitTypes eUnit;
	UnitTypes eBestUnit;
	bool bWarPlan;
	bool bDefense;
	bool bLandWar;
	bool bAssault;
	bool bPrimaryArea;
	bool bAreaAlone;
	bool bFinancialTrouble;
	bool bWarPossible;
	bool bDanger;
	int iHasMetCount;
	int iMilitaryWeight;
	int iCoastalCities = 0;
	int iBestValue;
	int iI;

	if (peBestUnitAI != NULL)
	{
		*peBestUnitAI = NO_UNITAI;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      11/30/08                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
/* original bts code
	pWaterArea = waterArea();
*/
	pWaterArea = waterArea(true);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	bDefense = (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
	bLandWar = (bDefense || (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bAssault = (area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT);
	bPrimaryArea = GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(area());
	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());
	bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();
	bWarPossible = GET_TEAM(getTeam()).AI_isWarPossible();
	bDanger = AI_isDanger();

	iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
	iMilitaryWeight = GET_PLAYER(getOwnerINLINE()).AI_militaryWeight(area());
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());

	if (pWaterArea != NULL)
	{
		iCoastalCities = GET_PLAYER(getOwnerINLINE()).countNumCoastalCitiesByArea(pWaterArea);
	}

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		aiUnitAIVal[iI] = 0;
	}

	if (!bFinancialTrouble && ((bPrimaryArea) ? (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0) : (area()->getBestFoundValue(getOwnerINLINE()) > 0)))
	{
		aiUnitAIVal[UNITAI_SETTLE]++;
	}

	aiUnitAIVal[UNITAI_WORKER] += GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area());

	aiUnitAIVal[UNITAI_ATTACK] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 7 : 12)) + ((bPrimaryArea && bWarPossible) ? 2 : 0) + 1);

	aiUnitAIVal[UNITAI_CITY_DEFENSE] += (iNumCitiesInArea + 1);
	aiUnitAIVal[UNITAI_CITY_COUNTER] += ((5 * (iNumCitiesInArea + 1)) / 8);
	aiUnitAIVal[UNITAI_CITY_SPECIAL] += ((iNumCitiesInArea + 1) / 2);

	if (bWarPossible)
	{
		aiUnitAIVal[UNITAI_ATTACK_CITY] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 10 : 17)) + ((bPrimaryArea) ? 1 : 0));
		aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 13 : 22)) + ((bPrimaryArea) ? 1 : 0));
		aiUnitAIVal[UNITAI_PARADROP] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 5 : 8)) + ((bPrimaryArea) ? 1 : 0));

		aiUnitAIVal[UNITAI_DEFENSE_AIR] += (GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);
		aiUnitAIVal[UNITAI_CARRIER_AIR] += GET_PLAYER(getOwnerINLINE()).AI_countCargoSpace(UNITAI_CARRIER_SEA);
		aiUnitAIVal[UNITAI_MISSILE_AIR] += GET_PLAYER(getOwnerINLINE()).AI_countCargoSpace(UNITAI_MISSILE_CARRIER_SEA);

		if (bPrimaryArea)
		{
			aiUnitAIVal[UNITAI_ICBM] += std::max((GET_PLAYER(getOwnerINLINE()).getTotalPopulation() / 25), ((GC.getGameINLINE().countCivPlayersAlive() + GC.getGameINLINE().countTotalNukeUnits()) / (GC.getGameINLINE().countCivPlayersAlive() + 1)));
		}
	}

	if (isBarbarian())
	{
		aiUnitAIVal[UNITAI_ATTACK] *= 2;
	}
	else
	{
		if (!bLandWar)
		{
			aiUnitAIVal[UNITAI_EXPLORE] += GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area());
		}

		if (pWaterArea != NULL)
		{
			aiUnitAIVal[UNITAI_WORKER_SEA] += AI_neededSeaWorkers();

			if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
			{
				if (bPrimaryArea)
				{
					aiUnitAIVal[UNITAI_EXPLORE_SEA] += GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea);
				}

				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0) && (pWaterArea->getNumTiles() > 300))
				{
					aiUnitAIVal[UNITAI_SETTLER_SEA]++;
				}

				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_MISSIONARY) > 0) && (pWaterArea->getNumTiles() > 400))
				{
					aiUnitAIVal[UNITAI_MISSIONARY_SEA]++;
				}

				if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SPY) > 0) && (pWaterArea->getNumTiles() > 500))
				{
					aiUnitAIVal[UNITAI_SPY_SEA]++;
				}

				aiUnitAIVal[UNITAI_PIRATE_SEA] += pWaterArea->getNumTiles() / 600;

				if (bWarPossible)
				{
					aiUnitAIVal[UNITAI_ATTACK_SEA] += std::min((pWaterArea->getNumTiles() / 150), ((((iCoastalCities * 2) + (iMilitaryWeight / 9)) / ((bAssault) ? 4 : 6)) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_RESERVE_SEA] += std::min((pWaterArea->getNumTiles() / 200), ((((iCoastalCities * 2) + (iMilitaryWeight / 7)) / 5) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_ESCORT_SEA] += (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) + (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_CARRIER_SEA) * 2));
					aiUnitAIVal[UNITAI_ASSAULT_SEA] += std::min((pWaterArea->getNumTiles() / 250), ((((iCoastalCities * 2) + (iMilitaryWeight / 6)) / ((bAssault) ? 5 : 8)) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_CARRIER_SEA] += std::min((pWaterArea->getNumTiles() / 350), ((((iCoastalCities * 2) + (iMilitaryWeight / 8)) / 7) + ((bPrimaryArea) ? 1 : 0)));
					aiUnitAIVal[UNITAI_MISSILE_CARRIER_SEA] += std::min((pWaterArea->getNumTiles() / 350), ((((iCoastalCities * 2) + (iMilitaryWeight / 8)) / 7) + ((bPrimaryArea) ? 1 : 0)));
				}
			}
		}

		if ((iHasMetCount > 0) && bWarPossible)
		{
			if (bLandWar || bAssault || !bFinancialTrouble || (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() == 0))
			{
				aiUnitAIVal[UNITAI_ATTACK] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 9 : 16)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_ATTACK_CITY] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 7 : 15)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_COLLATERAL] += ((iMilitaryWeight / ((bDefense) ? 8 : 14)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_PILLAGE] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 10 : 19)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_RESERVE] += ((iMilitaryWeight / ((bLandWar) ? 12 : 17)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 9 : 16)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
				aiUnitAIVal[UNITAI_PARADROP] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 4 : 8)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));

				aiUnitAIVal[UNITAI_ATTACK_AIR] += (GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);

				if (pWaterArea != NULL)
				{
					if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
					{
						aiUnitAIVal[UNITAI_ATTACK_SEA] += std::min((pWaterArea->getNumTiles() / 100), ((((iCoastalCities * 2) + (iMilitaryWeight / 10)) / ((bAssault) ? 5 : 7)) + ((bPrimaryArea) ? 1 : 0)));
						aiUnitAIVal[UNITAI_RESERVE_SEA] += std::min((pWaterArea->getNumTiles() / 150), ((((iCoastalCities * 2) + (iMilitaryWeight / 11)) / 8) + ((bPrimaryArea) ? 1 : 0)));
					}
				}
			}
		}
	}

	// XXX this should account for air and heli units too...
	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_unitAIDomainType((UnitAITypes)iI) == DOMAIN_SEA)
		{
			if (pWaterArea != NULL)
			{
				aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, ((UnitAITypes)iI));
			}
		}
		else if ((GET_PLAYER(getOwnerINLINE()).AI_unitAIDomainType((UnitAITypes)iI) == DOMAIN_AIR) || (iI == UNITAI_ICBM))
		{
			aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs((UnitAITypes)iI);
		}
		else
		{
			aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), ((UnitAITypes)iI));
		}
	}

	aiUnitAIVal[UNITAI_SETTLE] *= ((bDanger) ? 8 : 20);
	aiUnitAIVal[UNITAI_WORKER] *= ((bDanger) ? 2 : 7);
	aiUnitAIVal[UNITAI_ATTACK] *= 3;
	aiUnitAIVal[UNITAI_ATTACK_CITY] *= 4;
	aiUnitAIVal[UNITAI_COLLATERAL] *= 5;
	aiUnitAIVal[UNITAI_PILLAGE] *= 3;
	aiUnitAIVal[UNITAI_RESERVE] *= 3;
	aiUnitAIVal[UNITAI_COUNTER] *= 3;
	aiUnitAIVal[UNITAI_COUNTER] *= 2;
	aiUnitAIVal[UNITAI_CITY_DEFENSE] *= 2;
	aiUnitAIVal[UNITAI_CITY_COUNTER] *= 2;
	aiUnitAIVal[UNITAI_CITY_SPECIAL] *= 2;
	aiUnitAIVal[UNITAI_EXPLORE] *= ((bDanger) ? 6 : 15);
	aiUnitAIVal[UNITAI_ICBM] *= 18;
	aiUnitAIVal[UNITAI_WORKER_SEA] *= ((bDanger) ? 3 : 10);
	aiUnitAIVal[UNITAI_ATTACK_SEA] *= 5;
	aiUnitAIVal[UNITAI_RESERVE_SEA] *= 4;
	aiUnitAIVal[UNITAI_ESCORT_SEA] *= 20;
	aiUnitAIVal[UNITAI_EXPLORE_SEA] *= 18;
	aiUnitAIVal[UNITAI_ASSAULT_SEA] *= 14;
	aiUnitAIVal[UNITAI_SETTLER_SEA] *= 16;
	aiUnitAIVal[UNITAI_MISSIONARY_SEA] *= 12;
	aiUnitAIVal[UNITAI_SPY_SEA] *= 10;
	aiUnitAIVal[UNITAI_CARRIER_SEA] *= 8;
	aiUnitAIVal[UNITAI_MISSILE_CARRIER_SEA] *= 8;
	aiUnitAIVal[UNITAI_PIRATE_SEA] *= 5;
	aiUnitAIVal[UNITAI_ATTACK_AIR] *= 6;
	aiUnitAIVal[UNITAI_DEFENSE_AIR] *= 3;
	aiUnitAIVal[UNITAI_CARRIER_AIR] *= 15;
	aiUnitAIVal[UNITAI_MISSILE_AIR] *= 15;

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		aiUnitAIVal[iI] *= std::max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getUnitAIWeightModifier(iI) + 100));
		aiUnitAIVal[iI] /= 100;
	}

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		if (aiUnitAIVal[iI] > 0)
		{
			if (bAsync)
			{
				aiUnitAIVal[iI] += GC.getASyncRand().get(iMilitaryWeight, "AI Best UnitAI ASYNC");
			}
			else
			{
				aiUnitAIVal[iI] += GC.getGameINLINE().getSorenRandNum(iMilitaryWeight, "AI Best UnitAI");
			}

			if (aiUnitAIVal[iI] > iBestValue)
			{
				eUnit = AI_bestUnitAI(((UnitAITypes)iI), bAsync, eIgnoreAdvisor);

				if (eUnit != NO_UNIT)
				{
					iBestValue = aiUnitAIVal[iI];
					eBestUnit = eUnit;
					if (peBestUnitAI != NULL)
					{
						*peBestUnitAI = ((UnitAITypes)iI);
					}
				}
			}
		}
	}

	return eBestUnit;
}

/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
UnitTypes CvCityAI::AI_bestUnitAI(UnitAITypes eUnitAI, bool bAsync, AdvisorTypes eIgnoreAdvisor, bool bIgnoreNotUnitAIs)
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/
{
	UnitTypes eLoopUnit;
	UnitTypes eBestUnit;
	int iValue;
	int iBestValue;
	int iOriginalValue;
	int iBestOriginalValue;
	int iI, iJ, iK;
	

	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");

	bool bGrowMore = false;

	if (foodDifference() > 0)
	{
		// BBAI NOTE: This is where small city worker and settler production is blocked
		if (GET_PLAYER(getOwnerINLINE()).getNumCities() <= 2)
		{
			bGrowMore = ((getPopulation() < 3) && (AI_countGoodTiles(true, false, 100) >= getPopulation()));
		}
		else
		{
			bGrowMore = ((getPopulation() < 3) || (AI_countGoodTiles(true, false, 100) >= getPopulation()));
		}

		if (!bGrowMore && (getPopulation() < 6) && (AI_countGoodTiles(true, false, 80) >= getPopulation()))
		{
			if ((getFood() - (getFoodKept() / 2)) >= (growthThreshold() / 2))
			{
				if ((angryPopulation(1) == 0) && (healthRate(false, 1) == 0))
				{
					bGrowMore = true;
				}
			}
		}
	}
	iBestOriginalValue = 0;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getUnitInfo(eLoopUnit).getAdvisorType() != eIgnoreAdvisor))
			{
				if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
				{
				    
					if (!(bGrowMore && isFoodProduction(eLoopUnit)))
					{
						if (canTrain(eLoopUnit))
						{
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
							iOriginalValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area(), bIgnoreNotUnitAIs);
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

							if (iOriginalValue > iBestOriginalValue)
							{
								iBestOriginalValue = iOriginalValue;
							}
						}
					}
				}
			}
		}
	}

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getUnitInfo(eLoopUnit).getAdvisorType() != eIgnoreAdvisor))
			{
				if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
				{
				    
					if (!(bGrowMore && isFoodProduction(eLoopUnit)))
					{
						if (canTrain(eLoopUnit))
						{
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
							iValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area(), bIgnoreNotUnitAIs);
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

							if ((iValue > ((iBestOriginalValue * 2) / 3)) && ((eUnitAI != UNITAI_EXPLORE) || (iValue >= iBestOriginalValue)))
							{
								iValue *= (getProductionExperience(eLoopUnit) + 10);
								iValue /= 10;

                                //free promotions. slow?
                                //only 1 promotion per source is counted (ie protective isn't counted twice)
                                int iPromotionValue = 0;
                                //buildings
                                for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
                                {
                                    if (isFreePromotion((PromotionTypes)iJ) && !GC.getUnitInfo(eLoopUnit).getFreePromotions((PromotionTypes)iJ))
                                    {
                                        if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getPromotionInfo((PromotionTypes)iJ).getUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
                                        {
                                            iPromotionValue += 15;
                                            break;
                                        }
                                    }
                                }

                                //special to the unit
                                for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
                                {
                                    if (GC.getUnitInfo(eLoopUnit).getFreePromotions(iJ))
                                    {
                                        iPromotionValue += 15;
                                        break;
                                    }
                                }

                                //traits
                                for (iJ = 0; iJ < GC.getNumTraitInfos(); iJ++)
                                {
                                    if (hasTrait((TraitTypes)iJ))
                                    {
                                        for (iK = 0; iK < GC.getNumPromotionInfos(); iK++)
                                        {
                                            if (GC.getTraitInfo((TraitTypes) iJ).isFreePromotion(iK))
                                            {
                                                if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getTraitInfo((TraitTypes) iJ).isFreePromotionUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
                                                {
                                                    iPromotionValue += 15;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }

                                iValue *= (iPromotionValue + 100);
                                iValue /= 100;

								if (bAsync)
								{
									iValue *= (GC.getASyncRand().get(50, "AI Best Unit ASYNC") + 100);
									iValue /= 100;
								}
								else
								{
									iValue *= (GC.getGameINLINE().getSorenRandNum(50, "AI Best Unit") + 100);
									iValue /= 100;
								}


								int iBestHappy = 0;
								for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); ++iHurry)
								{
									if (canHurryUnit((HurryTypes)iHurry, eLoopUnit, true))
									{
										int iHappy = AI_getHappyFromHurry((HurryTypes)iHurry, eLoopUnit, true);
										if (iHappy > iBestHappy)
										{
											iBestHappy = iHappy;
										}
									}
								}

								if (0 == iBestHappy)
								{
									iValue += getUnitProduction(eLoopUnit);
								}

								iValue *= (GET_PLAYER(getOwnerINLINE()).getNumCities() * 2);
								iValue /= (GET_PLAYER(getOwnerINLINE()).getUnitClassCountPlusMaking((UnitClassTypes)iI) + GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);

								FAssert((MAX_INT / 1000) > iValue);
								iValue *= 1000;
						
								bool bIsSuicide = GC.getUnitInfo(eLoopUnit).isSuicide();
								
								if (bIsSuicide)
								{
									//much of this is compensated
									iValue /= 3;
								}

								if (0 == iBestHappy)
								{
									iValue /= std::max(1, (getProductionTurnsLeft(eLoopUnit, 0) + (GC.getUnitInfo(eLoopUnit).isSuicide() ? 1 : 4)));
								}
								else
								{
									iValue *= (2 + 3 * iBestHappy);
									iValue /= 100;
								}

								iValue = std::max(1, iValue);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestUnit = eLoopUnit;
								}
							}
						}
					}
				}
			}
		}
	}

	return eBestUnit;
}


BuildingTypes CvCityAI::AI_bestBuilding(int iFocusFlags, int iMaxTurns, bool bAsync, AdvisorTypes eIgnoreAdvisor)
{
	return AI_bestBuildingThreshold(iFocusFlags, iMaxTurns, /*iMinThreshold*/ 0, bAsync, eIgnoreAdvisor);
}

BuildingTypes CvCityAI::AI_bestBuildingThreshold(int iFocusFlags, int iMaxTurns, int iMinThreshold, bool bAsync, AdvisorTypes eIgnoreAdvisor)
{
	BuildingTypes eLoopBuilding;
	BuildingTypes eBestBuilding;
	bool bAreaAlone;
	int iProductionRank;
	int iTurnsLeft;
	int iValue;
	int iTempValue;
	int iBestValue;
	int iI, iJ;

	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestBuilding = NO_BUILDING;


	if (iFocusFlags & BUILDINGFOCUS_CAPITAL)
	{
		int iBestTurnsLeft = iMaxTurns > 0 ? iMaxTurns : MAX_INT;
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if (NO_BUILDING != eLoopBuilding)
			{
				CvBuildingInfo& kBuilding = GC.getBuildingInfo(eLoopBuilding);

				if (kBuilding.isCapital())
				{
					if (canConstruct(eLoopBuilding))
					{
						int iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

						if (iTurnsLeft <= iBestTurnsLeft)
						{
							eBestBuilding = eLoopBuilding;
							iBestTurnsLeft = iTurnsLeft;
						}
					}
				}
			}
		}

		return eBestBuilding;
	}

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (!(GET_PLAYER(getOwnerINLINE()).isBuildingClassMaxedOut(((BuildingClassTypes)iI), GC.getBuildingClassInfo((BuildingClassTypes)iI).getExtraPlayerInstances())))
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if ((eLoopBuilding != NO_BUILDING) && (getNumBuilding(eLoopBuilding) < GC.getCITY_MAX_NUM_BUILDINGS())
			&&  (!isProductionAutomated() || !(isWorldWonderClass((BuildingClassTypes)iI) || isNationalWonderClass((BuildingClassTypes)iI))))
			{
				//don't build wonders?
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/08/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
				// BBAI TODO: Temp testing, remove once centralized building is working
				bool bWonderOk = false;
				//if( isHuman() || getOwner()%2 == 1 )
				//{
					bWonderOk = ((iFocusFlags == 0) || (iFocusFlags & BUILDINGFOCUS_WONDEROK) || (iFocusFlags & BUILDINGFOCUS_WORLDWONDER));
				//}

				if( bWonderOk ||
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					!(isWorldWonderClass((BuildingClassTypes)iI) || 
					 isTeamWonderClass((BuildingClassTypes)iI) || 
					  isNationalWonderClass((BuildingClassTypes)iI) ||
					   isLimitedWonderClass((BuildingClassTypes)iI)))
				{
					if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getBuildingInfo(eLoopBuilding).getAdvisorType() != eIgnoreAdvisor))
					{
						if (canConstruct(eLoopBuilding))
						{
							iValue = AI_buildingValueThreshold(eLoopBuilding, iFocusFlags, iMinThreshold);

							if (GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass() != NO_BUILDINGCLASS)
							{
								BuildingTypes eFreeBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass());
								if (NO_BUILDING != eFreeBuilding)
								{
									iValue += (AI_buildingValue(eFreeBuilding, iFocusFlags) * (GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass())));
								}
							}
							if (isProductionAutomated())
							{
								for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
								{
									if (GC.getBuildingInfo(eLoopBuilding).getPrereqNumOfBuildingClass(iJ) > 0)
									{
										iValue = 0;
										break;
									}
								}
							}


							if (iValue > 0)
							{
								iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

								if (isWorldWonderClass((BuildingClassTypes)iI))
								{
									if (iProductionRank <= std::min(3, ((GET_PLAYER(getOwnerINLINE()).getNumCities() + 2) / 3)))
									{
										if (bAsync)
										{
											iTempValue = GC.getASyncRand().get(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand ASYNC");
										}
										else
										{
											iTempValue = GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
										}
										
										if (bAreaAlone)
										{
											iTempValue *= 2;
										}
										iValue += iTempValue;
									}
								}

								if (bAsync)
								{
									iValue *= (GC.getASyncRand().get(25, "AI Best Building ASYNC") + 100);
									iValue /= 100;
								}
								else
								{
									iValue *= (GC.getGameINLINE().getSorenRandNum(25, "AI Best Building") + 100);
									iValue /= 100;
								}

								iValue += getBuildingProduction(eLoopBuilding);
								
								
								bool bValid = ((iMaxTurns <= 0) ? true : false);
								if (!bValid)
								{
									bValid = (iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(iMaxTurns, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()));
								}
								if (!bValid)
								{
									for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); ++iHurry)
									{
										if (canHurryBuilding((HurryTypes)iHurry, eLoopBuilding, true))
										{
											if (AI_getHappyFromHurry((HurryTypes)iHurry, eLoopBuilding, true) > 0)
											{
												bValid = true;
												break;
											}
										}
									}
								}

								if (bValid)
								{
									FAssert((MAX_INT / 1000) > iValue);
									iValue *= 1000;
									iValue /= std::max(1, (iTurnsLeft + 3));

									iValue = std::max(1, iValue);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										eBestBuilding = eLoopBuilding;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return eBestBuilding;
}


int CvCityAI::AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags)
{
	return AI_buildingValueThreshold(eBuilding, iFocusFlags, 0);
}

// XXX should some of these count cities, buildings, etc. based on teams (because wonders are shared...)
// XXX in general, this function needs to be more sensitive to what makes this city unique (more likely to build airports if there already is a harbor...)
int CvCityAI::AI_buildingValueThreshold(BuildingTypes eBuilding, int iFocusFlags, int iThreshold)
{
	PROFILE_FUNC();

	int iTempValue;
	int iPass;
	int iI, iJ;

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	BuildingClassTypes eBuildingClass = (BuildingClassTypes) kBuilding.getBuildingClassType();
	int iLimitedWonderLimit = limitedWonderClassLimit(eBuildingClass);
	bool bIsLimitedWonder = (iLimitedWonderLimit >= 0);

	ReligionTypes eStateReligion = kOwner.getStateReligion();

	bool bAreaAlone = kOwner.AI_isAreaAlone(area());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                        jdog5000 & Fuyu       */
/*                                                                                              */
/* City AI, Bugfix                                                                              */
/************************************************************************************************/
	int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

	int iFoodDifference = foodDifference(false);

	// Reduce reaction to espionage induced happy/health problems
	int iHappinessLevel = happyLevel() - unhappyLevel(1) + getEspionageHappinessCounter()/2;
	int iAngryPopulation = range(-iHappinessLevel, 0, (getPopulation() + 1));
	int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, std::max(0, (iHappinessLevel + 1) / 2)) + getEspionageHealthCounter()/2;
	int iBadHealth = std::max(0, -iHealthLevel);

	int iHappyModifier = (iHappinessLevel <= iHealthLevel && iHappinessLevel <= 6) ? 6 : 3;
	int iHealthModifier = (iHealthLevel < iHappinessLevel && iHealthLevel <= 4) ? 4 : 2;
	if (iHappinessLevel >= 10)
	{
		iHappyModifier = 1;
	}
	if (iHealthLevel >= 8)
	{
		iHealthModifier = 0;
	}

	bool bProvidesPower = (kBuilding.isPower() || ((kBuilding.getPowerBonus() != NO_BONUS) && hasBonus((BonusTypes)(kBuilding.getPowerBonus()))) || kBuilding.isAreaCleanPower());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/********************************************************************************/
/* 	Alternative Building Evaluation				20.02.2010		Fuyu		    */
/* 	 - requires Actual Building Effects (BULL)								    */
/* 																			    */
/* 	TODO:																		*/
/* 		value riverPlotYieldChanges higher when we are already working on river	*/
/* 		add getBuildingYieldChange() ?										    */
/* 		more consistancy (values from yield changes should not differ so much	*/
/* 			depending on where they come from)									*/
/********************************************************************************/
	//Don't consider a building if it causes the city to immediately start shrinking from unhealthiness
//For that purpose ignore bad health and unhappiness from Espionage.
	int iBuildingActualHappiness = getAdditionalHappinessByBuilding(eBuilding);
	int iBuildingActualHealth = getAdditionalHealthByBuilding(eBuilding);
	int iBaseHappinessLevel = happyLevel() - unhappyLevel() + getEspionageHappinessCounter();
	int iBaseHealthLevel = goodHealth() - badHealth() + getEspionageHealthCounter();
	int iBaseFoodDifference = getYieldRate(YIELD_FOOD) - getPopulation()*GC.getFOOD_CONSUMPTION_PER_POPULATION() - std::max(0,-iBaseHealthLevel);
	int iTempHappinessLevel,iTempHealthLevel;

	//Temporary unhappiness that will mostly not be ignored
	int iTemporaryUnhappiness = 0;
	int iAngerPercent = 0;
	iAngerPercent += getHurryPercentAnger();
	iAngerPercent += getConscriptPercentAnger();
	iAngerPercent += getDefyResolutionPercentAnger();
	iAngerPercent += getWarWearinessPercentAnger();
	iTemporaryUnhappiness += ((iAngerPercent * (getPopulation())) / GC.getPERCENT_ANGER_DIVISOR());

	int iBadHealthFromBuilding = std::max(0,(-iBuildingActualHealth));
	int iUnhealthyPopulationFromBuilding = std::min(0,(-iBaseHealthLevel)) + iBadHealthFromBuilding;
//	int iTotalHealth = iBaseHealthLevel + iBuildingActualHealth;
	bool bShrinksWithPower = false;
//Allow a bit of shrinking: Population is expendable if angry, working a bad tile, or running a not-so-good specialist
	int iAllowedShrinkRate = GC.getFOOD_CONSUMPTION_PER_POPULATION() * (0
		+ std::max(0,-iBaseHappinessLevel-iTemporaryUnhappiness)
		+ std::min(1, std::max(0,(getWorkingPopulation() - AI_countGoodTiles(true, false, 50))))
		+ std::max(0,(visiblePopulation() - AI_countGoodSpecialists(false))));
	if (iUnhealthyPopulationFromBuilding > 0 && (iBaseFoodDifference + iAllowedShrinkRate < iUnhealthyPopulationFromBuilding ))
	{
		return 0;
	}
	else if (!(bProvidesPower || isPower()))  //if the city is still without power after building this
	{
		int iUnhealthyPopulationFromBuildingPlusPower = std::min(0,(badHealth() - goodHealth() - getEspionageHealthCounter())) + iBadHealthFromBuilding - GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE");
		if (iUnhealthyPopulationFromBuildingPlusPower > 0 && (iBaseFoodDifference + iAllowedShrinkRate < iUnhealthyPopulationFromBuildingPlusPower ))
		{
			bShrinksWithPower = true;
		}
	}
/********************************************************************************/
/* 	ABE END																	    */
/********************************************************************************/

	int iTotalPopulation = kOwner.getTotalPopulation();
	int iNumCities = kOwner.getNumCities();
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());
	
	int aiYieldRank[NUM_YIELD_TYPES];
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYieldRank[iI] = MAX_INT;
	}

	int aiCommerceRank[NUM_COMMERCE_TYPES];
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceRank[iI] = MAX_INT;
	}

	aiYieldRank[YIELD_PRODUCTION] = findBaseYieldRateRank(YIELD_PRODUCTION);
	bool bIsHighProductionCity = (aiYieldRank[YIELD_PRODUCTION] <= std::max(3, (iNumCities / 2)));
	
	int iCultureRank = findCommerceRateRank(COMMERCE_CULTURE);
	int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	bool bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	bool bCulturalVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1);
	bool bCulturalVictory2 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
	bool bCulturalVictory3 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3);

	bool bSpaceVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE1);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		

	bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();

	bool bForeignTrade = false;
	int iNumTradeRoutes = getTradeRoutes();
	for (iI = 0; iI < iNumTradeRoutes; ++iI)
	{
		CvCity* pTradeCity = getTradeCity(iI);
		if (NULL != pTradeCity)
		{
			if (GET_PLAYER(pTradeCity->getOwnerINLINE()).getTeam() != getTeam() || pTradeCity->area() != area())
			{
				bForeignTrade = true;
				break;
			}
		}
	}

	if (kBuilding.isCapital())
	{
		return 0;
	}

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (kBuilding.getReligionChange(iI) > 0)
		{
			if (!(GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI)))
			{
				return 0;
			}
		}
	}

	int iValue = 0;

	for (iPass = 0; iPass < 2; iPass++)
	{
		if ((iFocusFlags == 0) || (iValue > 0) || (iPass == 0))
		{
		    
		    if ((iFocusFlags & BUILDINGFOCUS_WORLDWONDER) || (iPass > 0))
		    {
		        if (isWorldWonderClass(eBuildingClass))
		        {
                    if (aiYieldRank[YIELD_PRODUCTION] <= 3)
                    {
						iValue++;
                    }
		        }
		    }
		    
			if ((iFocusFlags & BUILDINGFOCUS_DEFENSE) || (iPass > 0))
			{
				if (!bAreaAlone)
				{
					if ((GC.getGameINLINE().getBestLandUnit() == NO_UNIT) || !(GC.getUnitInfo(GC.getGameINLINE().getBestLandUnit()).isIgnoreBuildingDefense()))
					{
						iValue += (std::max(0, std::min(((kBuilding.getDefenseModifier() + getBuildingDefense()) - getNaturalDefense() - 10), kBuilding.getDefenseModifier())) / 4);
					}
				}

				iValue += kBuilding.getBombardDefenseModifier() / 8;
				
				iValue += -kBuilding.getAirModifier() / 4;
				iValue += -kBuilding.getNukeModifier() / 4;
				
				iValue += ((kBuilding.getAllCityDefenseModifier() * iNumCities) / 5);
				
				iValue += kBuilding.getAirlift() * 25;
			}

			if ((iFocusFlags & BUILDINGFOCUS_ESPIONAGE) || (iPass > 0))
			{
				iValue += kBuilding.getEspionageDefenseModifier() / 8;
			}

			if (((iFocusFlags & BUILDINGFOCUS_HAPPY) || (iPass > 0)) && !isNoUnhappiness())
			{
				int iBestHappy = 0;
				for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
				{
					if (canHurryBuilding((HurryTypes)iI, eBuilding, true))
					{
						int iHappyFromHurry = AI_getHappyFromHurry((HurryTypes)iI, eBuilding, true);
						if (iHappyFromHurry > iBestHappy)
						{
							iBestHappy = iHappyFromHurry;
						}
					}
				}
				iValue += iBestHappy * 10;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                              jdog5000        */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/

/********************************************************************************/
/* 	Alternative Building Evaluation				09.03.2010		Fuyu		    */
/********************************************************************************/
/* original Better AI code
				if( iBuildingActualHappiness < 0 )
				{
					// Building causes net decrease in city happiness
					iValue -= (-iBuildingActualHappiness + iAngryPopulation) * 6
						+ (-iBuildingActualHappiness) * iHappyModifier;

					// BBAI TODO: Check for potential shrink in population
					
				}
				else if( iBuildingActualHappiness > 0 )
				{
					// Building causes net increase in city happiness
					iValue += (std::min(iBuildingActualHappiness, iAngryPopulation) * 10) 
						+ (std::max(0, iBuildingActualHappiness - iAngryPopulation) * iHappyModifier);
				}
*/

				//Fuyu ToDo: How to handle Globe Theater national wonder?
				//For now just give massive boost if city is high food yet not one of the main production or commerce cities
				if (kBuilding.isNoUnhappiness() && bIsLimitedWonder)
				{
					iValue += (iAngryPopulation * 10) + getPopulation();
					aiYieldRank[YIELD_FOOD] = findBaseYieldRateRank(YIELD_FOOD);
					aiYieldRank[YIELD_COMMERCE] = findBaseYieldRateRank(YIELD_COMMERCE);
					if (aiYieldRank[YIELD_FOOD] <= (2 + iLimitedWonderLimit))
					{
						if (!(isNationalWonderClass(eBuildingClass)) ||
							((aiYieldRank[YIELD_PRODUCTION] > (2 + iLimitedWonderLimit)) 
                            && (aiYieldRank[YIELD_COMMERCE] > (2 + iLimitedWonderLimit))))
						{
							iValue += getPopulation() * 5;
						}
					}
				}
				else
				{
					iTempHealthLevel = iBaseHealthLevel - (getEspionageHealthCounter() / 2) + std::max(0, iBuildingActualHealth);
					iTempHappinessLevel = iBaseHappinessLevel - (getEspionageHappinessCounter() / 2);
					int iTempCurrentHappinessLevel = iBaseHappinessLevel;
					int iTempHappyChangeValue = 0;
					bool bIsNegative = false;
					if (iBuildingActualHappiness < 0)
					{
						bIsNegative = true;
						iBuildingActualHappiness = -iBuildingActualHappiness;
					}
					if (iBuildingActualHappiness != 0)
					{
						for (iI = 0; iI < iBuildingActualHappiness; iI++)
						{
							if (bIsNegative) { iTempHappinessLevel--; iTempCurrentHappinessLevel--; iTempHappyChangeValue += 1;}
							
							//The Value of Happiness
							if (iTempHappinessLevel < 0)
							{
								iTempHappyChangeValue += 10;
								if (bIsNegative) {
									iTempHappyChangeValue += 9;
									if (iTempCurrentHappinessLevel < 0)
									{
										iTempHappyChangeValue += 10;
									}								
								} 
							}
							else if (iTempHappinessLevel <= iTempHealthLevel && iTempHappinessLevel <= 6) {
								iTempHappyChangeValue += 6;
								if (bIsNegative) { iTempHappyChangeValue += 3;}
							}
							else if (iTempHappinessLevel < 10 ) { iTempHappyChangeValue += 3; }
							else { iTempHappyChangeValue += 1; }
							
							if (!bIsNegative) { iTempHappinessLevel++; iTempCurrentHappinessLevel++; }
						}
						
						if (bIsNegative)
						{
							iValue -= iTempHappyChangeValue;
						}
						else 
						{
							iValue += iTempHappyChangeValue;
						}
					}
				}
/********************************************************************************/
/* 	ABE END																	    */
/********************************************************************************/

				iValue += (-kBuilding.getHurryAngerModifier() * getHurryPercentAnger()) / 100;

				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iValue += (kBuilding.getCommerceHappiness(iI) * iHappyModifier) / 4;
				}

				int iWarWearinessModifer = kBuilding.getWarWearinessModifier();
				if (iWarWearinessModifer != 0)
				{
					iValue += (-iWarWearinessModifer * iHappyModifier) / 16;
				}

				iValue += (kBuilding.getAreaHappiness() * (iNumCitiesInArea - 1) * 8);
				iValue += (kBuilding.getGlobalHappiness() * (iNumCities - 1) * 8);

				int iWarWearinessPercentAnger = kOwner.getWarWearinessPercentAnger();
				int iGlobalWarWearinessModifer = kBuilding.getGlobalWarWearinessModifier();
				if (iGlobalWarWearinessModifer != 0)
				{
					iValue += (-(((iGlobalWarWearinessModifer * iWarWearinessPercentAnger / 100) / GC.getPERCENT_ANGER_DIVISOR())) * (iNumCities - 1));
					iValue += (-iGlobalWarWearinessModifer * iHappyModifier) / 16;
				}

				CvCivilizationInfo& kCivilization = GC.getCivilizationInfo(getCivilizationType());
				for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
				{
					int iBuildingHappinessChanges = kBuilding.getBuildingHappinessChanges(iI);
					if (iBuildingHappinessChanges != 0)
					{
						BuildingTypes eLoopBuilding = (BuildingTypes)kCivilization.getCivilizationBuildings(iI);
						if (eLoopBuilding != NO_BUILDING)
						{
							iValue += (iBuildingHappinessChanges * (kOwner.getBuildingClassCount((BuildingClassTypes)iI) - getNumBuilding(eLoopBuilding)) * 8);
						}
						else
						{
							iValue += (iBuildingHappinessChanges * kOwner.getBuildingClassCount((BuildingClassTypes)iI) * 8);
						}
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			}

			if (((iFocusFlags & BUILDINGFOCUS_HEALTHY) || (iPass > 0)) && !isNoUnhealthyPopulation())
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                          jdog5000 & Fuyu     */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/

/********************************************************************************/
/* 	Alternative Building Evaluation				09.03.2010		Fuyu		    */
/********************************************************************************/
/* original Better AI code
				if( iBuildingActualHealth < 0 )
				{
					// Building causes net decrease in city health
					iValue -= (-iBuildingActualHealth + iBadHealth) * 6
						+ (-iBuildingActualHealth) * iHealthModifier;

					// BBAI TODO: Check for potential shrink in population
					
				}
				else if( iBuildingActualHealth > 0 )
				{
					// Building causes net increase in city health
					iValue += (std::min(iBuildingActualHealth, iBadHealth) * 10)
						+ (std::max(0, iBuildingActualHealth - iBadHealth) * iHealthModifier);
				}
*/

				iTempHappinessLevel = iBaseHappinessLevel - (getEspionageHappinessCounter() / 2) + std::max(0, iBuildingActualHappiness);
				iTempHealthLevel = (goodHealth() - badHealth(/*bNoAngry*/ false, std::max(0, (iTempHappinessLevel + 1) / 2))) + ((getEspionageHealthCounter() + 1) / 2 ); 
				int iTempCurrentHealthLevel = iBaseHealthLevel;
				int iTempHealthChangeValue = 0;
				bool bIsNegative = false;
				if (iBuildingActualHealth < 0)
				{
					bIsNegative = true;
					iBuildingActualHealth = -iBuildingActualHealth;
				}
				if (iBuildingActualHealth != 0)
				{
					for (iI = 0; iI < iBuildingActualHealth; iI++)
					{
						if (bIsNegative) { iTempHealthLevel--; iTempCurrentHealthLevel--; iTempHealthChangeValue += 1;}
					
						//Health Values
						if (iTempCurrentHealthLevel < 0) {
							iTempHealthChangeValue += 10;
							if (bIsNegative) {
								iTempHealthChangeValue += 9;
								if(std::max(0,iBaseFoodDifference-1) < -iTempCurrentHealthLevel)
								{
									iTempHealthChangeValue += 10;
								}									
							}
						}
						else if (iTempHealthLevel < 0) { iTempHealthChangeValue += 8; }
						else if (iTempHealthLevel < iTempHappinessLevel && iTempHealthLevel <= 4) {
							iTempHealthChangeValue += 4;
							if (bIsNegative) { iTempHealthChangeValue += 1;}
						}
						else if (iTempHealthLevel < 8 ) { iTempHealthChangeValue += 2; }
						else if (!bIsNegative) { iTempHealthChangeValue += 1; }
					
						if (!bIsNegative) { iTempHealthLevel++; iTempCurrentHealthLevel++; }
					}
					if (bIsNegative)
					{
						iValue -= iTempHealthChangeValue;
					}
					else
					{
						iValue += iTempHealthChangeValue;
					}
				}
/********************************************************************************/
/* 	ABE END																	    */
/********************************************************************************/

				iValue += (kBuilding.getAreaHealth() * (iNumCitiesInArea - 1 ) * 4);
				iValue += (kBuilding.getGlobalHealth() * (iNumCities - 1) * 4);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			}

			if ((iFocusFlags & BUILDINGFOCUS_EXPERIENCE) || (iPass > 0))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 12 : 6));

				for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
				{
					if (canTrain((UnitCombatTypes)iI))
					{
						iValue += (kBuilding.getUnitCombatFreeExperience(iI) * ((iHasMetCount > 0) ? 6 : 3));
					}
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					int iDomainExpValue = 0;
					if (iI == DOMAIN_SEA)
					{
						iDomainExpValue = 7;
					} 
					else if (iI == DOMAIN_LAND)
					{
						iDomainExpValue = 12;
					}
					else
					{
						iDomainExpValue = 6;
					}
					iValue += (kBuilding.getDomainFreeExperience(iI) * ((iHasMetCount > 0) ? iDomainExpValue : iDomainExpValue / 2));
				}
			}
			
			// since this duplicates BUILDINGFOCUS_EXPERIENCE checks, do not repeat on pass 1
			if ((iFocusFlags & BUILDINGFOCUS_DOMAINSEA))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 16 : 8));

				for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
				{
					UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);

					if (NO_UNIT != eUnit)
					{
						CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);
						int iCombatType = kUnitInfo.getUnitCombatType();
						if (kUnitInfo.getDomainType() == DOMAIN_SEA && canTrain(eUnit) && iCombatType != NO_UNITCOMBAT)
						{
							iValue += (kBuilding.getUnitCombatFreeExperience(iCombatType) * ((iHasMetCount > 0) ? 6 : 3));
						}
					}
				}

				iValue += (kBuilding.getDomainFreeExperience(DOMAIN_SEA) * ((iHasMetCount > 0) ? 16 : 8));

				iValue += (kBuilding.getDomainProductionModifier(DOMAIN_SEA) / 4);
			}

			if ((iFocusFlags & BUILDINGFOCUS_MAINTENANCE) || (iFocusFlags & BUILDINGFOCUS_GOLD) || (iPass > 0))
			{

				int iBaseMaintenance = getMaintenanceTimes100();
				int iExistingUpkeep = (iBaseMaintenance * std::max(0, 100 + getMaintenanceModifier())) / 100;
				int iNewUpkeep = (iBaseMaintenance * std::max(0, 100 + getMaintenanceModifier() + kBuilding.getMaintenanceModifier())) / 100;
				
				iTempValue = (iExistingUpkeep - iNewUpkeep) / 16;
				
				if (bFinancialTrouble)
				{
					iTempValue *= 2;
				}

				iValue += iTempValue;
			}

			if ((iFocusFlags & BUILDINGFOCUS_SPECIALIST) || (iPass > 0))
			{
				int iSpecialistsValue = 0;
				int iCurrentSpecialistsRunnable = 0;
				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (iI != GC.getDefineINT("DEFAULT_SPECIALIST"))
					{
						bool bUnlimited = (GET_PLAYER(getOwnerINLINE()).isSpecialistValid((SpecialistTypes)iI));
						int iRunnable = (getMaxSpecialistCount((SpecialistTypes)iI) > 0);
						
						if (bUnlimited || (iRunnable > 0))
						{
							if (bUnlimited)
							{
								iCurrentSpecialistsRunnable += 5;
							}
							else
							{
								iCurrentSpecialistsRunnable += iRunnable;
							}
						}
						
						
						if (kBuilding.getSpecialistCount(iI) > 0)
						{
							if ((!bUnlimited) && (iRunnable < 5))
							{
								iTempValue = AI_specialistValue(((SpecialistTypes)iI), false, false);

								iTempValue *= (20 + (40 * kBuilding.getSpecialistCount(iI)));
								iTempValue /= 100;

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       01/09/10                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
								if (iFoodDifference < 2)
								{
									iValue /= 4;
								}
								if (iRunnable > 0)
								{
									iValue /= 1 + iRunnable;
								}
*/
								if (iFoodDifference < 2)
								{
									iTempValue /= 4;
								}
								if (iRunnable > 0)
								{
									iTempValue /= 1 + iRunnable;
								}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

								iSpecialistsValue += std::max(12, (iTempValue / 100));
							}
						}
					}
				}
				
				if (iSpecialistsValue > 0)
				{
					iValue += iSpecialistsValue / std::max(2, iCurrentSpecialistsRunnable);
				}
			}
			
			if ((iFocusFlags & (BUILDINGFOCUS_GOLD | BUILDINGFOCUS_RESEARCH)) || iPass > 0)
			{
				// trade routes
				iTempValue = ((kBuilding.getTradeRoutes() * ((8 * std::max(0, (totalTradeModifier() + 100))) / 100))
								* (getPopulation() / 5 + 1));
				int iGlobalTradeValue = (((6 * iTotalPopulation) / 5) / iNumCities);
				iTempValue += (kBuilding.getCoastalTradeRoutes() * kOwner.countNumCoastalCities() * iGlobalTradeValue);
				iTempValue += (kBuilding.getGlobalTradeRoutes() * iNumCities * iGlobalTradeValue);
				
				iTempValue += ((kBuilding.getTradeRouteModifier() * getTradeYield(YIELD_COMMERCE)) / (bForeignTrade ? 12 : 25));
				if (bForeignTrade)
				{
					iTempValue += ((kBuilding.getForeignTradeRouteModifier() * getTradeYield(YIELD_COMMERCE)) / 12);
				}
					
				if (bFinancialTrouble)
				{
					iTempValue *= 2;
				}
					
				if (kOwner.isNoForeignTrade())
				{
					iTempValue /= 3;
				}


				iValue += iTempValue;
			}

			if (iPass > 0)
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                       jdog5000 & Afforess    */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
				if (kBuilding.isAreaCleanPower() && !(area()->isCleanPower(getTeam())))
				{
					int iLoop;
					for( CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop) )
					{
						if( pLoopCity->area() == area() )
						{
							if( pLoopCity->isDirtyPower() )
							{
								//Fuyu: less value for cities that don't already have power
								iValue += 8;
							}
							else if( !(pLoopCity->isPower()) )
							{
								iValue += 12;
							}
						}
					}
				}

				if (kBuilding.getDomesticGreatGeneralRateModifier() != 0)
				{
					iValue += (kBuilding.getDomesticGreatGeneralRateModifier() / 10);
				}

				if (kBuilding.isAreaBorderObstacle() && !(area()->isBorderObstacle(getTeam())))
				{
					if( !GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS) )
					{
						iValue += (iNumCitiesInArea);

						if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
						{
							iValue += (iNumCitiesInArea);
						}
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				if (kBuilding.isGovernmentCenter())
				{
					FAssert(!(kBuilding.isCapital()));
					iValue += ((calculateDistanceMaintenance() - 3) * iNumCitiesInArea);
				}

				if (kBuilding.isMapCentering())
				{
					iValue++;
				}

				if (kBuilding.getFreeBonus() != NO_BONUS)
				{
					iValue += (kOwner.AI_bonusVal((BonusTypes)(kBuilding.getFreeBonus()), 1) *
						         ((kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getFreeBonus())) == 0) ? 2 : 1) *
						         (iNumCities + kBuilding.getNumFreeBonuses()));
				}

				if (kBuilding.getNoBonus() != NO_BONUS)
				{
					iValue -= kOwner.AI_bonusVal((BonusTypes)kBuilding.getNoBonus());
				}

				if (kBuilding.getFreePromotion() != NO_PROMOTION)
				{
					iValue += ((iHasMetCount > 0) ? 100 : 40); // XXX some sort of promotion value???
				}

				if (kBuilding.getCivicOption() != NO_CIVICOPTION)
				{
					for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
					{
						if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == kBuilding.getCivicOption())
						{
							if (!(kOwner.canDoCivics((CivicTypes)iI)))
							{
								iValue += (kOwner.AI_civicValue((CivicTypes)iI) / 10);
							}
						}
					}
				}
				
				int iGreatPeopleRateModifier = kBuilding.getGreatPeopleRateModifier();
				if (iGreatPeopleRateModifier > 0)
				{
					int iGreatPeopleRate = getBaseGreatPeopleRate();
					const int kTargetGPRate = 10;

					// either not a wonder, or a wonder and our GP rate is at least the target rate
					if (!bIsLimitedWonder || iGreatPeopleRate >= kTargetGPRate)
					{
						iValue += ((iGreatPeopleRateModifier * iGreatPeopleRate) / 16);
					}
					// otherwise, this is a limited wonder (aka National Epic), we _really_ do not want to build this here
					// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
					else
					{
						iValue -= ((iGreatPeopleRateModifier * (kTargetGPRate - iGreatPeopleRate)) / 12);
					}
				}

				iValue += ((kBuilding.getGlobalGreatPeopleRateModifier() * iNumCities) / 8);

				iValue += (-(kBuilding.getAnarchyModifier()) / 4);

				iValue += (-(kBuilding.getGlobalHurryModifier()) * 2);

				iValue += (kBuilding.getGlobalFreeExperience() * iNumCities * ((iHasMetCount > 0) ? 6 : 3));

				if (bCanPopRush)
				{
					iValue += kBuilding.getFoodKept() / 2;
				}

				iValue += kBuilding.getAirlift() * (getPopulation() * 3 + 10);
				
				int iAirDefense = -kBuilding.getAirModifier();
				if (iAirDefense > 0)
				{
					if (((kOwner.AI_totalUnitAIs(UNITAI_DEFENSE_AIR) > 0) && (kOwner.AI_totalUnitAIs(UNITAI_ATTACK_AIR) > 0)) || (kOwner.AI_totalUnitAIs(UNITAI_MISSILE_AIR) > 0))
					{
						iValue += iAirDefense / ((iHasMetCount > 0) ? 2 : 4);
					}
				}

				iValue += kBuilding.getAirUnitCapacity() * (getPopulation() * 2 + 10);

				iValue += (-(kBuilding.getNukeModifier()) / ((iHasMetCount > 0) ? 10 : 20));

				iValue += (kBuilding.getFreeSpecialist() * 16);
				iValue += (kBuilding.getAreaFreeSpecialist() * iNumCitiesInArea * 12);
				iValue += (kBuilding.getGlobalFreeSpecialist() * iNumCities * 12);

				iValue += ((kBuilding.getWorkerSpeedModifier() * kOwner.AI_getNumAIUnits(UNITAI_WORKER)) / 10);

				int iMilitaryProductionModifier = kBuilding.getMilitaryProductionModifier();
				if (iHasMetCount > 0 && iMilitaryProductionModifier > 0)
				{
					// either not a wonder, or a wonder and we are a high production city
					if (!bIsLimitedWonder || bIsHighProductionCity)
					{
						iValue += (iMilitaryProductionModifier / 4);

						// if a wonder, then pick one of the best cities
						if (bIsLimitedWonder)
						{
							// if one of the top 3 production cities, give a big boost
							if (aiYieldRank[YIELD_PRODUCTION] <= (2 + iLimitedWonderLimit))
							{
								iValue += (2 * iMilitaryProductionModifier) / (2 + aiYieldRank[YIELD_PRODUCTION]);
							}
						}
						// otherwise, any of the top half of cities will do
						else if (bIsHighProductionCity)
						{
							iValue += iMilitaryProductionModifier / 4;
						}
						iValue += ((iMilitaryProductionModifier * (getFreeExperience() + getSpecialistFreeExperience())) / 10);
					}
					// otherwise, this is a limited wonder (aka Heroic Epic), we _really_ do not want to build this here
					// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
					else
					{
						iValue -= (iMilitaryProductionModifier * aiYieldRank[YIELD_PRODUCTION]) / 5;
					}
				}

				iValue += (kBuilding.getSpaceProductionModifier() / 5);
				iValue += ((kBuilding.getGlobalSpaceProductionModifier() * iNumCities) / 20);
				

				if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
				{
					iValue++; // XXX improve this for diversity...
				}
				
				// prefer to build great people buildings in places that already have some GP points
				iValue += (kBuilding.getGreatPeopleRateChange() * 10) * (1 + (getBaseGreatPeopleRate() / 2));

				if (!bAreaAlone)
				{
					iValue += (kBuilding.getHealRateChange() / 2);
				}

				iValue += (kBuilding.getGlobalPopulationChange() * iNumCities * 4);

				iValue += (kBuilding.getFreeTechs() * 80);

				iValue += kBuilding.getEnemyWarWearinessModifier() / 2;

				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (kBuilding.getFreeSpecialistCount(iI) > 0)
					{
						iValue += ((AI_specialistValue(((SpecialistTypes)iI), false, false) * kBuilding.getFreeSpecialistCount(iI)) / 50);
					}
				}

				for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
				{
					if (kBuilding.getImprovementFreeSpecialist(iI) > 0)
					{
						iValue += kBuilding.getImprovementFreeSpecialist(iI) * countNumImprovedPlots((ImprovementTypes)iI, true) * 50;
					}
				}

				for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					iValue += (kBuilding.getDomainProductionModifier(iI) / 5);
					
					if (bIsHighProductionCity)
					{
						iValue += (kBuilding.getDomainProductionModifier(iI) / 5);
					}
				}

				for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
				{
					if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
					{
						// BBAI TODO: Smarter monastary construction, better support for mods

						if (kOwner.AI_totalAreaUnitAIs(area(), ((UnitAITypes)(GC.getUnitInfo((UnitTypes)iI).getDefaultUnitAIType()))) == 0)
						{
							iValue += iNumCitiesInArea;
						}

						iValue++;

						ReligionTypes eReligion = (ReligionTypes)(GC.getUnitInfo((UnitTypes)iI).getPrereqReligion());
						if (eReligion != NO_RELIGION)
						{
							//encouragement to get some minimal ability to train special units
							if (bCulturalVictory1 || isHolyCity(eReligion) || isCapital())
							{
								iValue += (2 + iNumCitiesInArea);
							}
							
							if (bCulturalVictory2 && GC.getUnitInfo((UnitTypes)iI).getReligionSpreads(eReligion))
							{
								//this gives a very large extra value if the religion is (nearly) unique
								//to no extra value for a fully spread religion.
								//I'm torn between huge boost and enough to bias towards the best monastery type.
								int iReligionCount = GET_PLAYER(getOwnerINLINE()).getHasReligionCount(eReligion);
								iValue += (100 * (iNumCities - iReligionCount)) / (iNumCities * (iReligionCount + 1));
							}
						}
					}
				}
				
				// is this building needed to build other buildings?
				for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(((BuildingTypes) iI), eBuildingClass);

					// if we need some of us to build iI building, and we dont need more than we have cities
					if (iPrereqBuildings > 0 && iPrereqBuildings <= iNumCities)
					{
						// do we need more than what we are currently building?
						if (iPrereqBuildings > kOwner.getBuildingClassCountPlusMaking(eBuildingClass))
						{
							iValue += (iNumCities * 3);

							if (bCulturalVictory1)
							{
								BuildingTypes eLoopBuilding = (BuildingTypes) iI;
								CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
								int iLoopBuildingCultureModifier = kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE);
								if (iLoopBuildingCultureModifier > 0)
								{
									int iLoopBuildingsBuilt = kOwner.getBuildingClassCount((BuildingClassTypes) kLoopBuilding.getBuildingClassType());
									
									// if we have less than the number needed in culture cities
									//		OR we are one of the top cities and we do not have the building
									if (iLoopBuildingsBuilt < iCulturalVictoryNumCultureCities || 
										(iCultureRank <= iCulturalVictoryNumCultureCities && 0 == getNumBuilding(eLoopBuilding)))
									{
										iValue += iLoopBuildingCultureModifier;

										if (bCulturalVictory3)
										{
											iValue += iLoopBuildingCultureModifier * 2;
										}
									}
								}
							}
						}
					}
				}
				
				for (int iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
				{
					if (kBuilding.getVoteSourceType() == iI)
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/24/10                              jdog5000        */
/*                                                                                              */
/* City AI, Victory Strategy AI                                                                 */
/************************************************************************************************/					
						iTempValue = 0;
						if (kBuilding.isStateReligion())
						{
							int iShareReligionCount = 0;
							int iPlayerCount = 0;
							for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
							{
								CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
								if ((iPlayer != getOwner()) && kLoopPlayer.isAlive())
								{
									iPlayerCount++;
									if (GET_PLAYER(getOwnerINLINE()).getStateReligion() == kLoopPlayer.getStateReligion())
									{
										iShareReligionCount++;										
									}
								}
							}
							iTempValue += (200 * (1 + iShareReligionCount)) / (1 + iPlayerCount);
						}
						else
						{
							iTempValue += 100;
						}

						iValue += (iTempValue * (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY1) ? 5 : 1));
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				}

			}

			if (iPass > 0)
			{
				for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iTempValue = 0;

					iValue += ((kBuilding.getTradeRouteModifier() * getTradeYield((YieldTypes)iI)) / 12);
					if (bForeignTrade)
					{
						iValue += ((kBuilding.getForeignTradeRouteModifier() * getTradeYield((YieldTypes)iI)) / 12);
					}

					if (iFoodDifference > 0)
					{
						iValue += kBuilding.getFoodKept() / 2;
					}

					if (kBuilding.getSeaPlotYieldChange(iI) > 0)
					{
					    iTempValue += kBuilding.getSeaPlotYieldChange(iI) * AI_buildingSpecialYieldChangeValue(eBuilding, (YieldTypes)iI);
					}
					if (kBuilding.getRiverPlotYieldChange(iI) > 0)
					{
						iTempValue += (kBuilding.getRiverPlotYieldChange(iI) * countNumRiverPlots() * 4);
					}
					iTempValue += (kBuilding.getGlobalSeaPlotYieldChange(iI) * kOwner.countNumCoastalCities() * 8);
					iTempValue += (kBuilding.getYieldChange(iI) * 6);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
					iTempValue += (getBuildingYieldChange(eBuildingClass, (YieldTypes)iI) * 5);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					iTempValue += ((kBuilding.getYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / 10);
/********************************************************************************/
/* 	Alternative Building Evaluation				18/01/10		Fuyu		    */
/********************************************************************************/
/* original code
					iTempValue += ((kBuilding.getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 12 : 15));
*/
					if (!bShrinksWithPower)
					{
						iTempValue += ((kBuilding.getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 10 : 17));
					}
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					iTempValue += ((kBuilding.getAreaYieldModifier(iI) * iNumCitiesInArea) / 3);
					iTempValue += ((kBuilding.getGlobalYieldModifier(iI) * iNumCities) / 3);

					if (bProvidesPower && !isPower())
					{
						iTempValue += ((getPowerYieldRateModifier((YieldTypes)iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
					}

					for (iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
					{
						iTempValue += ((kBuilding.getSpecialistYieldChange(iJ, iI) * kOwner.getTotalPopulation()) / 5);
					}

					for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += ((kBuilding.getBonusYieldModifier(iJ, iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
						}
					}
					
					if (iTempValue != 0)
					{
						if (bFinancialTrouble && iI == YIELD_COMMERCE)
						{
							iTempValue *= 2;
						}

						iTempValue *= kOwner.AI_yieldWeight((YieldTypes)iI);
						iTempValue /= 100;
						
						if (aiYieldRank[iI] == MAX_INT)
						{
							aiYieldRank[iI] = findBaseYieldRateRank((YieldTypes) iI);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						if (bIsLimitedWonder && (aiYieldRank[iI] > (3 + iLimitedWonderLimit)))
						{
							iTempValue *= -1;
						}

						iValue += iTempValue;
					}
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_FOOD)
				{

					iValue += kBuilding.getFoodKept();

					if (kBuilding.getSeaPlotYieldChange(YIELD_FOOD) > 0)
					{
					    
					    iTempValue = kBuilding.getSeaPlotYieldChange(YIELD_FOOD) * AI_buildingSpecialYieldChangeValue(eBuilding, YIELD_FOOD);
					    if ((iTempValue < 8) && (getPopulation() > 3))
					    {
					        // don't bother
					    }
					    else
					    {
                            iValue += ((iTempValue * 4) / std::max(2, iFoodDifference));
					    }
					}

					if (kBuilding.getRiverPlotYieldChange(YIELD_FOOD) > 0)
					{
						iValue += (kBuilding.getRiverPlotYieldChange(YIELD_FOOD) * countNumRiverPlots() * 4);
					}
				}

				if (iFocusFlags & BUILDINGFOCUS_PRODUCTION)
				{
					iTempValue = ((kBuilding.getYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 20);
/********************************************************************************/
/* 	Alternative Building Evaluation				18/01/10		Fuyu		    */
/********************************************************************************/
/* original code
					iTempValue += ((kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / ((bProvidesPower || isPower()) ? 24 : 30));

*/
					//Fuyu Shrink consideration; and "? 24 : 30" is not enough / too much -> "? 20 : 35" (full value if there is already power, less if you still have to build a power plant. No value if you can't build any powerplant without shrinking the city)
					if (!bShrinksWithPower)
					{
						iTempValue += ((kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / ((bProvidesPower || isPower()) ? 20 : 35));
					}
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					if (kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) > 0)
					{
						int iNumWaterPlots = countNumWaterPlots();
						if (!bIsLimitedWonder || (iNumWaterPlots > NUM_CITY_PLOTS / 2))
						{
					    	iTempValue += kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) * iNumWaterPlots;
						}
					}
					if (kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) > 0)
					{
						iTempValue += (kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) * countNumRiverPlots() * 4);
					}
					if (bProvidesPower && !isPower())
					{
/********************************************************************************/
/* 	Alternative Building Evaluation				18/01/10		Fuyu		    */
/********************************************************************************/
/* original code
						iTempValue += ((getPowerYieldRateModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 12);

*/
						//Fuyu "/ 12" is too much. -> "/ 20"
						iTempValue += ((getPowerYieldRateModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 20);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					}
					
					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiYieldRank[YIELD_PRODUCTION] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}

				if (iFocusFlags & BUILDINGFOCUS_GOLD)
				{
					iTempValue = ((kBuilding.getYieldModifier(YIELD_COMMERCE) * getBaseYieldRate(YIELD_COMMERCE)));
					iTempValue *= kOwner.getCommercePercent(COMMERCE_GOLD);
					
					if (bFinancialTrouble)
					{
						iTempValue *= 2;
					}
					
					iTempValue /= 3000;
					
					if (MAX_INT == aiCommerceRank[COMMERCE_GOLD])
					{
						aiCommerceRank[COMMERCE_GOLD] = findCommerceRateRank(COMMERCE_GOLD);
					}

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_GOLD] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;
				}
			}

			if (iPass > 0)
			{
				for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iTempValue = 0;

					iTempValue += (kBuilding.getCommerceChange(iI) * 4);
					iTempValue += (kBuilding.getObsoleteSafeCommerceChange(iI) * 4);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
					iTempValue += (getBuildingCommerceChange(eBuildingClass, (CommerceTypes)iI) * 3);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					iTempValue *= 100 + kBuilding.getCommerceModifier(iI);
					iTempValue /= 100;
					
					if ((CommerceTypes)iI == COMMERCE_CULTURE)
					{
					    if (bCulturalVictory1)
					    {
					        iTempValue *= 2;					        
					    }
					}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/25/10                              jdog5000        */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
					if ((CommerceTypes)iI == COMMERCE_RESEARCH)
					{
					    if (bSpaceVictory1)
					    {
					        iTempValue *= 3;		
							iTempValue /= 2;
					    }
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

					if (kBuilding.getCommerceChangeDoubleTime(iI) > 0)
					{
						if ((kBuilding.getCommerceChange(iI) > 0) || (kBuilding.getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue += (1000 / kBuilding.getCommerceChangeDoubleTime(iI));
						}
					}
					
					// add value for a commerce modifier
					int iCommerceModifier = kBuilding.getCommerceModifier(iI);
					int iBaseCommerceRate = getBaseCommerceRate((CommerceTypes) iI);
					int iCommerceMultiplierValue = iCommerceModifier * iBaseCommerceRate;
					if (((CommerceTypes) iI) == COMMERCE_CULTURE && iCommerceModifier != 0)
					{
						if (bCulturalVictory1)
						{							
							// if this is one of our top culture cities, then we want to build this here first!
							if (iCultureRank <= iCulturalVictoryNumCultureCities)
							{					        
								iCommerceMultiplierValue /= 8;					        
								
								// if we at culture level 3, then these need to get built asap
								if (bCulturalVictory3)
								{
									// its most important to build in the lowest rate city, but important everywhere
									iCommerceMultiplierValue += std::max(100, 500 - iBaseCommerceRate) * iCommerceModifier;					        
								}
							}
							else
							{
								int iCountBuilt = kOwner.getBuildingClassCountPlusMaking(eBuildingClass);

								// do we have enough buildings to build extras?
								bool bHaveEnough = true;

								// if its limited and the limit is less than the number we need in culture cities, do not build here
								if (bIsLimitedWonder && (iLimitedWonderLimit <= iCulturalVictoryNumCultureCities))
								{
									bHaveEnough = false;
								}

								for (iJ = 0; bHaveEnough && iJ < GC.getNumBuildingClassInfos(); iJ++)
								{
									// count excess the number of prereq buildings which do not have this building built for yet
									int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(eBuilding, (BuildingClassTypes) iJ, -iCountBuilt);
									
									// do we not have enough built (do not count ones in progress)
									if (iPrereqBuildings > 0 && kOwner.getBuildingClassCount((BuildingClassTypes) iJ) <  iPrereqBuildings)
									{
										bHaveEnough = false;
									}
								}

								// if we have enough and our rank is close to the top, then possibly build here too
								if (bHaveEnough && (iCultureRank - iCulturalVictoryNumCultureCities) <= 3)
								{
									iCommerceMultiplierValue /= 12;
								}
								// otherwise, we really do not want to build this here
								else
								{
									iCommerceMultiplierValue /= 30;
								}
							}
						}
						else
						{
							iCommerceMultiplierValue /= 15;

							// increase priority if we need culture oppressed city
							iCommerceMultiplierValue *= (100 - calculateCulturePercent(getOwnerINLINE()));
						}
					}
					else
					{
 						iCommerceMultiplierValue /= 15;
					}
					iTempValue += iCommerceMultiplierValue;

					iTempValue += ((kBuilding.getGlobalCommerceModifier(iI) * iNumCities) / 4);
					iTempValue += ((kBuilding.getSpecialistExtraCommerce(iI) * kOwner.getTotalPopulation()) / 3);

					if (eStateReligion != NO_RELIGION)
					{
						iTempValue += (kBuilding.getStateReligionCommerce(iI) * kOwner.getHasReligionCount(eStateReligion) * 3);
					}

					if (kBuilding.getGlobalReligionCommerce() != NO_RELIGION)
					{
						iTempValue += (GC.getReligionInfo((ReligionTypes)(kBuilding.getGlobalReligionCommerce())).getGlobalReligionCommerce(iI) * GC.getGameINLINE().countReligionLevels((ReligionTypes)(kBuilding.getGlobalReligionCommerce())) * 2);
						if (eStateReligion == (ReligionTypes)(kBuilding.getGlobalReligionCommerce()))
						{
						    iTempValue += 10;
						}
					}

					CorporationTypes eCorporation = (CorporationTypes)kBuilding.getFoundsCorporation();
					int iCorpValue = 0;
					if (NO_CORPORATION != eCorporation)
					{
						iCorpValue = kOwner.AI_corporationValue(eCorporation, this);
						
						for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); iCorp++)
						{
							if (iCorp != eCorporation)
							{
								if (kOwner.hasHeadquarters((CorporationTypes)iCorp))
								{
									if (GC.getGame().isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
									{
										if (kOwner.AI_corporationValue((CorporationTypes)iCorp, this) > iCorpValue)
										{
											iCorpValue = -1;
											break;											
										}
										else
										{
											if (!isHasCorporation((CorporationTypes)iCorp))
											{
												iCorpValue = -1;
											}
										}
									}
								}
							}
						}
						
						iTempValue += iCorpValue / 100;
					}
					
					if (iCorpValue >= 0)//Don't build if it'll hurt us.
					{
						if (kBuilding.getGlobalCorporationCommerce() != NO_CORPORATION)
						{
							int iGoldValue = (GC.getCorporationInfo((CorporationTypes)(kBuilding.getGlobalCorporationCommerce())).getHeadquarterCommerce(iI) * GC.getGameINLINE().countCorporationLevels((CorporationTypes)(kBuilding.getGlobalCorporationCommerce())) * 2);
							
							iGoldValue += GC.getCorporationInfo((CorporationTypes)(kBuilding.getGlobalCorporationCommerce())).getHeadquarterCommerce(iI);
							if (iGoldValue > 0)
							{
								iGoldValue += 2 + (iNumCities / 4); 
								iGoldValue += std::min(iGoldValue, getBuildingCommerce(COMMERCE_GOLD) / 2) / 2;							
							}
							iGoldValue *= 2;
							iGoldValue *= getTotalCommerceRateModifier(COMMERCE_GOLD);
							iGoldValue *= std::max(50, getTotalCommerceRateModifier(COMMERCE_GOLD) - 150);
							iGoldValue /= 5000;
							iCorpValue += iGoldValue;
						}
					}
					
					if (iCorpValue > 0)
					{
						if (kOwner.isNoCorporations())
						{
							iCorpValue /= 2;
						}
						iValue += iCorpValue;
					}

					if (kBuilding.isCommerceFlexible(iI))
					{
						if (!(kOwner.isCommerceFlexible((CommerceTypes)iI)))
						{
							iTempValue += 40;
						}
					}

					if (kBuilding.isCommerceChangeOriginalOwner(iI))
					{
						if ((kBuilding.getCommerceChange(iI) > 0) || (kBuilding.getObsoleteSafeCommerceChange(iI) > 0))
						{
							iTempValue++;
						}
					}
					
					if (iTempValue != 0)
					{
						if (bFinancialTrouble && iI == COMMERCE_GOLD)
						{
							iTempValue *= 2;
						}

						iTempValue *= kOwner.AI_commerceWeight(((CommerceTypes)iI), this);
						iTempValue = (iTempValue + 99) / 100;

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						if (MAX_INT == aiCommerceRank[iI])
						{
							aiCommerceRank[iI] = findCommerceRateRank((CommerceTypes) iI);
						}
						if (bIsLimitedWonder && ((aiCommerceRank[iI] > (3 + iLimitedWonderLimit)))
							|| (bCulturalVictory1 && (iI == COMMERCE_CULTURE) && (aiCommerceRank[iI] == 1)))
						{
							iTempValue *= -1;

							// for culture, just set it to zero, not negative, just about every wonder gives culture
							if (iI == COMMERCE_CULTURE)
							{
								iTempValue = 0;
							}
						}
						iValue += iTempValue;
					}
				}

				for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (kBuilding.getReligionChange(iI) > 0)
					{
						if (GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
						{
							iValue += (kBuilding.getReligionChange(iI) * ((eStateReligion == iI) ? 10 : 1));
						}
					}
				}

				if (NO_VOTESOURCE != kBuilding.getVoteSourceType())
				{
					iValue += 100;
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_GOLD)
				{
					iTempValue = ((kBuilding.getCommerceModifier(COMMERCE_GOLD) * getBaseCommerceRate(COMMERCE_GOLD)) / 40);
					
					if (iTempValue != 0)
					{
						if (bFinancialTrouble)
						{
							iTempValue *= 2;
						}
						
						if (MAX_INT == aiCommerceRank[COMMERCE_GOLD])
						{
							aiCommerceRank[COMMERCE_GOLD] = findCommerceRateRank(COMMERCE_GOLD);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_GOLD] > (3 + iLimitedWonderLimit)))
						{
							iTempValue *= -1;
						}
						iValue += iTempValue;
					}
					
					iValue += (kBuilding.getCommerceChange(COMMERCE_GOLD) * 4);
					iValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_GOLD) * 4);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
					iValue += (getBuildingCommerceChange(eBuildingClass, COMMERCE_GOLD) * 3);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
				}

				if (iFocusFlags & BUILDINGFOCUS_RESEARCH)
				{
					iTempValue = ((kBuilding.getCommerceModifier(COMMERCE_RESEARCH) * getBaseCommerceRate(COMMERCE_RESEARCH)) / 40);
					
					if (iTempValue != 0)
					{
						if (MAX_INT == aiCommerceRank[COMMERCE_RESEARCH])
						{
							aiCommerceRank[COMMERCE_RESEARCH] = findCommerceRateRank(COMMERCE_RESEARCH);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_RESEARCH] > (3 + iLimitedWonderLimit)))
						{
							iTempValue *= -1;
						}

						iValue += iTempValue;
					}
					iValue += (kBuilding.getCommerceChange(COMMERCE_RESEARCH) * 4);
					iValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_RESEARCH) * 4);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
					iValue += (getBuildingCommerceChange(eBuildingClass, COMMERCE_RESEARCH) * 3);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
				}

				if (iFocusFlags & BUILDINGFOCUS_CULTURE)
				{
					iTempValue = (kBuilding.getCommerceChange(COMMERCE_CULTURE) * 3);
					iTempValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) * 3);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
					iTempValue += (getBuildingCommerceChange(eBuildingClass, COMMERCE_CULTURE) * 2);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
					{
						iTempValue += (kBuilding.getCommerceChange(COMMERCE_ESPIONAGE) * 3);
						iTempValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_ESPIONAGE) * 3);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
						iTempValue += (getBuildingCommerceChange(eBuildingClass, COMMERCE_ESPIONAGE) * 2);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					}

					if ((getCommerceRate(COMMERCE_CULTURE) == 0) && (AI_calculateTargetCulturePerTurn() == 1))
					{
						if (iTempValue >= 3)
						{
							iTempValue += 7;
						}						
					}
					
					if (iTempValue != 0)
					{
						if (MAX_INT == aiCommerceRank[COMMERCE_CULTURE])
						{
							aiCommerceRank[COMMERCE_CULTURE] = findCommerceRateRank(COMMERCE_CULTURE);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, 
						// do not count the culture value
						// we probably do not want to build this here (but we might)
						if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_CULTURE] > (3 + iLimitedWonderLimit)))
						{
							iTempValue  = 0;
						}
						iValue += iTempValue;
					}
					
					iValue += ((kBuilding.getCommerceModifier(COMMERCE_CULTURE) * getBaseCommerceRate(COMMERCE_CULTURE)) / 15);
					if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
					{
						iValue += ((kBuilding.getCommerceModifier(COMMERCE_ESPIONAGE) * getBaseCommerceRate(COMMERCE_ESPIONAGE)) / 15);
					}
				}
				
                if (iFocusFlags & BUILDINGFOCUS_BIGCULTURE)
				{
					iTempValue = (kBuilding.getCommerceModifier(COMMERCE_CULTURE) / 5);
					if (iTempValue != 0)
					{
						if (MAX_INT == aiCommerceRank[COMMERCE_CULTURE])
						{
							aiCommerceRank[COMMERCE_CULTURE] = findCommerceRateRank(COMMERCE_CULTURE);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, 
						// do not count the culture value
						// we probably do not want to build this here (but we might)
						if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_CULTURE] > (3 + iLimitedWonderLimit)))
						{
							iTempValue  = 0;
						}

						iValue += iTempValue;
					}
				}
				
				if (iFocusFlags & BUILDINGFOCUS_ESPIONAGE || (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE) && (iFocusFlags & BUILDINGFOCUS_CULTURE)))
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/09/10                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
					iTempValue = ((kBuilding.getCommerceModifier(COMMERCE_ESPIONAGE) * getBaseCommerceRate(COMMERCE_ESPIONAGE)) / 80);
					
					if (iTempValue != 0)
					{
						if (MAX_INT == aiCommerceRank[COMMERCE_ESPIONAGE])
						{
							aiCommerceRank[COMMERCE_ESPIONAGE] = findCommerceRateRank(COMMERCE_ESPIONAGE);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_ESPIONAGE] > (3 + iLimitedWonderLimit)))
						{
							iTempValue *= -1;
						}

						iValue += iTempValue;
					}
					iTempValue = (kBuilding.getCommerceChange(COMMERCE_ESPIONAGE) * 1);
					iTempValue += (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_ESPIONAGE) * 1);
/********************************************************************************/
/* 	Alternative Building Evaluation				24.03.2010		Fuyu		    */
/********************************************************************************/
					iTempValue += (getBuildingCommerceChange(eBuildingClass, COMMERCE_ESPIONAGE) * 1);
/********************************************************************************/
/* ABE END																	    */
/********************************************************************************/
					iTempValue *= 100 + kBuilding.getCommerceModifier(COMMERCE_ESPIONAGE);
					iValue += iTempValue / 100;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				}
			}
			
			if ((iThreshold > 0) && (iPass == 0))
			{
				if (iValue < iThreshold)
				{
					iValue = 0;
				}
			}

			if (iPass > 0 && !isHuman())
			{
				iValue += kBuilding.getAIWeight();
				if (iValue > 0)
				{
					for (iI = 0; iI < GC.getNumFlavorTypes(); iI++)
					{
						iValue += (kOwner.AI_getFlavorValue((FlavorTypes)iI) * kBuilding.getFlavorValue(iI));
					}
				}
			}
		}
	}
//	
//	// obsolete checks
//	bool bCanResearchObsoleteTech = false;
//	int iErasUntilObsolete = MAX_INT;
//	if (kBuilding.getObsoleteTech() != NO_TECH)
//	{
//		TechTypes eObsoleteTech = (TechTypes) kBuilding.getObsoleteTech();
//		FAssertMsg(eObsoleteTech == NO_TECH || !(GET_TEAM(getTeam()).isHasTech(eObsoleteTech)), "Team expected to not have the tech that obsoletes eBuilding");
//		iErasUntilObsolete = GC.getTechInfo(eObsoleteTech).getEra() - kOwner.getCurrentEra();
//		bCanResearchObsoleteTech = kOwner.canResearch(eObsoleteTech);
//	}
//
//	if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
//	{
//		TechTypes eSpecialBldgObsoleteTech = (TechTypes) GC.getSpecialBuildingInfo((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType())).getObsoleteTech();
//		FAssertMsg(eSpecialBldgObsoleteTech == NO_TECH || !(GET_TEAM(getTeam()).isHasTech(eSpecialBldgObsoleteTech)), "Team expected to not have the tech that obsoletes eBuilding");
//		if (eSpecialBldgObsoleteTech != NO_TECH)
//		{
//			int iSpecialBldgErasUntilObsolete = GC.getTechInfo(eSpecialBldgObsoleteTech).getEra() - kOwner.getCurrentEra();
//
//			if (iSpecialBldgErasUntilObsolete < iErasUntilObsolete)
//			{
//				iErasUntilObsolete = iSpecialBldgErasUntilObsolete;
//			}
//
//			if (!bCanResearchObsoleteTech)
//			{
//				bCanResearchObsoleteTech = kOwner.canResearch(eSpecialBldgObsoleteTech);
//			}
//		}
//	}
//	
//	// tech path method commented out, it would be more accurate if we want to be so.
//	//int iObsoleteTechPathLength = 0;
//	//if (kBuilding.getObsoleteTech() != NO_TECH)
//	//{
//	//	iObsoleteTechPathLength = findPathLength(kBuilding.getObsoleteTech(), false);
//	//}
//
//	//if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
//	//{
//	//	TechTypes eSpecialBldgObsoleteTech = GC.getSpecialBuildingInfo((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType())).getObsoleteTech();
//	//	int iSpecialBldgObsoleteTechPathLength = findPathLength(eSpecialBldgObsoleteTech, false);
//
//	//	if (iSpecialBldgObsoleteTechPathLength < iObsoleteTechPathLength)
//	//	{
//	//		iObsoleteTechPathLength = iSpecialBldgObsoleteTechPathLength;
//	//	}
//	//}
//	
//	// if we can research obsolete tech, then this almost no value
//	if (bCanResearchObsoleteTech)
//	{
//		iValue = std::min(16, (iValue + 31) / 32);
//	}
//	// if we are going obsolete in the next era, the current era, or even previous eras...
//	else if (iErasUntilObsolete < 2)
//	{
//		// do not care about obsolete if we are going cultural and there is a culture benefit post obsolesence
//		if (!bCulturalVictory1 || (kBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) <= 0))
//		{
//			iValue++;
//			iValue /= 2;
//		}
//	}

//	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
//	{
//		if (!GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBuildingInfo((BuildingTypes)iI).getObsoleteTech())))
//		{
//			if (GC.getBuildingInfo((BuildingTypes)iI).isBuildingClassNeededInCity(kBuilding.getBuildingClassType()))
//			{
//				iValue += AI_buildingValue((BuildingTypes)iI, iFocusFlags);
//			}
//
//			int iNumPrereqs = kOwner.getBuildingClassPrereqBuilding((BuildingTypes)iI, (BuildingClassTypes)kBuilding.getBuildingClassType()) - kOwner.getBuildingClassCount((BuildingClassTypes)kBuilding.getBuildingClassType());
//			if (iNumPrereqs > 0)
//			{
//				iValue += AI_buildingValue((BuildingTypes)iI, iFocusFlags) / iNumPrereqs;
//			}
//		}
//	}

	if (!canConstruct(eBuilding))
	{
		//This building is being constructed in some special way, 
		//reduce the value for small cities.
		if (getPopulation() < 6)
		{
			iValue /= (8 - getPopulation());
		}
		
	}

	return std::max(0, iValue);
}


ProjectTypes CvCityAI::AI_bestProject()
{
	ProjectTypes eBestProject;
	int iProductionRank;
	int iTurnsLeft;
	int iValue;
	int iBestValue;
	int iI;

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestProject = NO_PROJECT;

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (canCreate((ProjectTypes)iI))
		{
			iValue = AI_projectValue((ProjectTypes)iI);

			if ((GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialUnit() != NO_SPECIALUNIT) ||
				  (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialBuilding() != NO_SPECIALBUILDING) ||
				  GC.getProjectInfo((ProjectTypes)iI).isAllowsNukes())
			{
				if (GC.getGameINLINE().getSorenRandNum(100, "Project Everyone") == 0)
				{
					iValue++;
				}
			}

			if (iValue > 0)
			{
				iValue += getProjectProduction((ProjectTypes)iI);

				iTurnsLeft = getProductionTurnsLeft(((ProjectTypes)iI), 0);

				if ((iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(10, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent())) || !(GET_TEAM(getTeam()).isHuman()))
				{
					if ((iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(20, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent())) || (iProductionRank <= std::max(3, (GET_PLAYER(getOwnerINLINE()).getNumCities() / 2))))
					{
						if (iProductionRank == 1)
						{
							iValue += iTurnsLeft;
						}
						else
						{
							FAssert((MAX_INT / 1000) > iValue);
							iValue *= 1000;
							iValue /= std::max(1, (iTurnsLeft + 10));
						}

						iValue = std::max(1, iValue);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestProject = ((ProjectTypes)iI);
						}
					}
				}
			}
		}
	}

	return eBestProject;
}


int CvCityAI::AI_projectValue(ProjectTypes eProject)
{
	int iValue;
	int iI;

	iValue = 0;

	if (GC.getProjectInfo(eProject).getNukeInterception() > 0)
	{
		if (GC.getGameINLINE().canTrainNukes())
		{
			iValue += (GC.getProjectInfo(eProject).getNukeInterception() / 10);
		}
	}

	if (GC.getProjectInfo(eProject).getTechShare() > 0)
	{
		if (GC.getProjectInfo(eProject).getTechShare() < GET_TEAM(getTeam()).getHasMetCivCount(true))
		{
			iValue += (20 / GC.getProjectInfo(eProject).getTechShare());
		}
	}

	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes)iI))
		{
			iValue += (std::max(0, (GC.getProjectInfo(eProject).getVictoryThreshold(iI) - GET_TEAM(getTeam()).getProjectCount(eProject))) * 20);
		}
	}

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		iValue += (std::max(0, (GC.getProjectInfo((ProjectTypes)iI).getProjectsNeeded(eProject) - GET_TEAM(getTeam()).getProjectCount(eProject))) * 10);
	}

	return iValue;
}


ProcessTypes CvCityAI::AI_bestProcess()
{
	return AI_bestProcess(NO_COMMERCE);
}

ProcessTypes CvCityAI::AI_bestProcess(CommerceTypes eCommerceType)
{
	ProcessTypes eBestProcess;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestProcess = NO_PROCESS;

	for (iI = 0; iI < GC.getNumProcessInfos(); iI++)
	{
		if (canMaintain((ProcessTypes)iI))
		{
			iValue = AI_processValue((ProcessTypes)iI, eCommerceType);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestProcess = ((ProcessTypes)iI);
			}
		}
	}

	return eBestProcess;
}


int CvCityAI::AI_processValue(ProcessTypes eProcess)
{
	return AI_processValue(eProcess, NO_COMMERCE);
}

int CvCityAI::AI_processValue(ProcessTypes eProcess, CommerceTypes eCommerceType)
{
	int iValue;
	int iTempValue;
	int iI;
	bool bValid = (eCommerceType == NO_COMMERCE);
	
	iValue = 0;

	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_GOLD);
	}

	// if we own less than 50%, or we need to pop borders
	if ((plot()->calculateCulturePercent(getOwnerINLINE()) < 50) || (getCultureLevel() <= (CultureLevelTypes) 1))
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_CULTURE);
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/30/09                                jdog5000      */
/*                                                                                              */
/* Cultural Victory AI                                                                          */
/************************************************************************************************/
	if ( GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) )
	{
		// Final city for cultural victory will build culture to speed up victory
		if( findCommerceRateRank(COMMERCE_CULTURE) == GC.getGameINLINE().culturalVictoryNumCultureCities() )
		{
			iValue += 2*GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_CULTURE);
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iTempValue = GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI);
		if (!bValid && ((CommerceTypes)iI == eCommerceType) && (iTempValue > 0))
		{
			bValid = true;
		    iTempValue *= 2;
		}

		iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);
		
		iTempValue /= 100;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/08/09                                jdog5000      */
/*                                                                                              */
/* Gold AI                                                                                      */
/************************************************************************************************/
		iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI);

		iTempValue /= 60;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

		iValue += iTempValue;
	}

	return (bValid ? iValue : 0);
}


int CvCityAI::AI_neededSeaWorkers()
{
	CvArea* pWaterArea;
	int iNeededSeaWorkers = 0;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/01/09                                jdog5000      */
/*                                                                                              */
/* Worker AI                                                                                    */
/************************************************************************************************/
	pWaterArea = waterArea(true);
	
	if (pWaterArea == NULL)
	{
		return 0;
	}

	iNeededSeaWorkers += GET_PLAYER(getOwnerINLINE()).countUnimprovedBonuses(pWaterArea, plot());

	// Check if second water area city can reach was any unimproved bonuses
	pWaterArea = secondWaterArea();
	if (pWaterArea != NULL)
	{
		iNeededSeaWorkers += GET_PLAYER(getOwnerINLINE()).countUnimprovedBonuses(pWaterArea, plot());
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	return iNeededSeaWorkers;
}


bool CvCityAI::AI_isDefended(int iExtra)
{
	PROFILE_FUNC();

	return ((plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) + iExtra) >= AI_neededDefenders()); // XXX check for other team's units?
}

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						10/17/08		jdog5000		*/
/* 																			*/
/* 	Air AI																	*/
/********************************************************************************/
/* original BTS code
bool CvCityAI::AI_isAirDefended(int iExtra)
{
	PROFILE_FUNC();
	
	return ((plot()->plotCount(PUF_canAirDefend, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isDomainType, DOMAIN_AIR) + iExtra) >= AI_neededAirDefenders()); // XXX check for other team's units?
}
*/
// Function now answers question of whether city has enough ready air defense, no longer just counts fighters
bool CvCityAI::AI_isAirDefended(bool bCountLand, int iExtra)
{
	PROFILE_FUNC();
	
	int iAirDefenders = iExtra;
	int iAirIntercept = 0;
	int iLandIntercept = 0;

	CvUnit* pLoopUnit;
	CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);

		if ((pLoopUnit->getOwnerINLINE() == getOwnerINLINE()))
		{
			if ( pLoopUnit->canAirDefend() )
			{
				if( pLoopUnit->getDomainType() == DOMAIN_AIR )
				{
					// can find units which are already air patrolling using group activity
					if( pLoopUnit->getGroup()->getActivityType() == ACTIVITY_INTERCEPT )
					{
						iAirIntercept += pLoopUnit->currInterceptionProbability();
					}
					else
					{
						// Count air units which can air patrol
						if( pLoopUnit->getDamage() == 0 && !pLoopUnit->hasMoved() )
						{
							if( pLoopUnit->AI_getUnitAIType() == UNITAI_DEFENSE_AIR )
							{
								iAirIntercept += pLoopUnit->currInterceptionProbability();
							}
							else
							{
								iAirIntercept += pLoopUnit->currInterceptionProbability()/3;
							}
						}

					}
				}
				else if( pLoopUnit->getDomainType() == DOMAIN_LAND )
				{
					iLandIntercept += pLoopUnit->currInterceptionProbability();
				}
			}
		}
	}

	iAirDefenders += (iAirIntercept/100);

	if( bCountLand )
	{
		iAirDefenders += (iLandIntercept/100);
	}

	int iNeededAirDefenders = AI_neededAirDefenders();
	bool bHaveEnough = (iAirDefenders >= iNeededAirDefenders);

	return bHaveEnough;
}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								*/
/********************************************************************************/


/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/25/10                                jdog5000      */
/*                                                                                              */
/* War strategy AI, Barbarian AI                                                                */
/************************************************************************************************/
int CvCityAI::AI_neededDefenders()
{
	PROFILE_FUNC();
	int iDefenders;
	bool bOffenseWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bool bDefenseWar = ((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE));
	
	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 1;
	}

	if (isBarbarian())
	{
		iDefenders = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianInitialDefenders();
		iDefenders += ((getPopulation() + 2) / 7);
		return iDefenders;
	}

	iDefenders = 1;
	
	if (hasActiveWorldWonder() || isCapital() || isHolyCity())
	{
		iDefenders++;

		if( GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_ALERT1) || GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
		{
			iDefenders++;
		}
	}
	
	if (!GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_CRUSH))
	{
		iDefenders += AI_neededFloatingDefenders();
	}
	else
	{
		iDefenders += (AI_neededFloatingDefenders() + 2) / 4;
	}
	
	if (bDefenseWar || GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_ALERT2))
	{
		if (!(plot()->isHills()))
		{
			iDefenders++;
		}
	}
	
	if ((GC.getGame().getGameTurn() - getGameTurnAcquired()) < 10)
	{
/* original code
		if (bOffenseWar)
		{
			if (!hasActiveWorldWonder() && !isHolyCity())
			{
				iDefenders /= 2;
				iDefenders = std::max(1, iDefenders);
			}
		}		
	}
	
	if (GC.getGame().getGameTurn() - getGameTurnAcquired() < 10)
	{
		iDefenders = std::max(2, iDefenders);
		if (AI_isDanger())
		{
			iDefenders ++;
		}
		if (bDefenseWar)
		{
			iDefenders ++;
		}
*/
		iDefenders = std::max(2, iDefenders);

		if (bOffenseWar && getTotalDefense(true) > 0)
		{
			if (!hasActiveWorldWonder() && !isHolyCity())
			{
				iDefenders /= 2;
			}
		}		
		
		if (AI_isDanger())
		{
			iDefenders++;
		}
		if (bDefenseWar)
		{
			iDefenders++;
		}
	}
	
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_LAST_STAND))
	{
		iDefenders += 10;
	}

	if( GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) )
	{
		if( findCommerceRateRank(COMMERCE_CULTURE) <= GC.getGameINLINE().culturalVictoryNumCultureCities() )
		{
			iDefenders += 4;

			if( bDefenseWar )
			{
				iDefenders += 2;
			}
		}
	}

	if( GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE3) )
	{
		if( isCapital() || isProductionProject())
		{
			iDefenders += 4;

			if( bDefenseWar )
			{
				iDefenders += 3;
			}
		}

		if( isCapital() && GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE4) )
		{
			iDefenders += 6;
		}
	}
	
	iDefenders = std::max(iDefenders, AI_minDefenders());

	return iDefenders;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

int CvCityAI::AI_minDefenders()
{
	int iDefenders = 1;
	int iEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
	if (iEra > 0)
	{
		iDefenders++;
	}
	if (((iEra - GC.getGame().getStartEra() / 2) >= GC.getNumEraInfos() / 2) && isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
	{
		iDefenders++;
	}
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
	if (getProductionUnitAI() == UNITAI_SETTLE)
	{
		iDefenders++;
	}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

	return iDefenders;
}
	
int CvCityAI::AI_neededFloatingDefenders()
{
	if (m_iNeededFloatingDefendersCacheTurn != GC.getGame().getGameTurn())
	{
		AI_updateNeededFloatingDefenders();
	}
	return m_iNeededFloatingDefenders;	
}

void CvCityAI::AI_updateNeededFloatingDefenders()
{
	int iFloatingDefenders = GET_PLAYER(getOwnerINLINE()).AI_getTotalFloatingDefendersNeeded(area());
		
	int iTotalThreat = std::max(1, GET_PLAYER(getOwnerINLINE()).AI_getTotalAreaCityThreat(area()));
	
	iFloatingDefenders -= area()->getCitiesPerPlayer(getOwnerINLINE());
	
	iFloatingDefenders *= AI_cityThreat();
	iFloatingDefenders += (iTotalThreat / 2);
	iFloatingDefenders /= iTotalThreat;
	
	m_iNeededFloatingDefenders = iFloatingDefenders;
	m_iNeededFloatingDefendersCacheTurn = GC.getGame().getGameTurn();
}

int CvCityAI::AI_neededAirDefenders()
{
	int iDefenders;

	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 0;
	}

	iDefenders = 0;

	int iRange = 5;

	int iOtherTeam = 0;
	int iEnemyTeam = 0;
	for (int iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (int iDY = -(iRange); iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if ((pLoopPlot != NULL) && pLoopPlot->isOwned() && (pLoopPlot->getTeam() != getTeam()))
			{
				iOtherTeam++;
				if (GET_TEAM(getTeam()).AI_getWarPlan(pLoopPlot->getTeam()) != NO_WARPLAN)
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/01/09                                jdog5000      */
/*                                                                                              */
/* Air AI                                                                                       */
/************************************************************************************************/
					// If enemy has no bombers, don't need to defend as much
					if( GET_PLAYER(pLoopPlot->getOwner()).AI_totalUnitAIs(UNITAI_ATTACK_AIR) == 0 )
					{
						continue;
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					iEnemyTeam += 2;
					if (pLoopPlot->isCity())
					{
						iEnemyTeam += 6;
					}
				}
			}
		}
	}
	
	iDefenders += (iOtherTeam + iEnemyTeam + 2) / 8;

	iDefenders = std::min((iEnemyTeam > 0) ? 4 : 2, iDefenders);
	
	if (iDefenders == 0)
	{
		if (isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
		{
			iDefenders++;
		}
	}
	return iDefenders;
}


bool CvCityAI::AI_isDanger()
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* City AI, Efficiency                                                                          */
/************************************************************************************************/
	//return GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2, false);
	return GET_PLAYER(getOwnerINLINE()).AI_getAnyPlotDanger(plot(), 2, false);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
}


int CvCityAI::AI_getEmphasizeAvoidGrowthCount()
{
	return m_iEmphasizeAvoidGrowthCount;
}


bool CvCityAI::AI_isEmphasizeAvoidGrowth()
{
	return (AI_getEmphasizeAvoidGrowthCount() > 0);
}


int CvCityAI::AI_getEmphasizeGreatPeopleCount()
{
	return m_iEmphasizeGreatPeopleCount;
}


bool CvCityAI::AI_isEmphasizeGreatPeople()
{
	return (AI_getEmphasizeGreatPeopleCount() > 0);
}


bool CvCityAI::AI_isAssignWorkDirty()
{
	return m_bAssignWorkDirty;
}


void CvCityAI::AI_setAssignWorkDirty(bool bNewValue)
{
	m_bAssignWorkDirty = bNewValue;
}


bool CvCityAI::AI_isChooseProductionDirty()
{
	return m_bChooseProductionDirty;
}


void CvCityAI::AI_setChooseProductionDirty(bool bNewValue)
{
	m_bChooseProductionDirty = bNewValue;
}


CvCity* CvCityAI::AI_getRouteToCity() const
{
	return getCity(m_routeToCity);
}


void CvCityAI::AI_updateRouteToCity()
{
	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	gDLL->getFAStarIFace()->ForceReset(&GC.getRouteFinder());

	iBestValue = MAX_INT;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (pLoopCity != this)
				{
					if (pLoopCity->area() == area())
					{
						if (!(gDLL->getFAStarIFace()->GeneratePath(&GC.getRouteFinder(), getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), false, getOwnerINLINE(), true)))
						{
							iValue = plotDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								pBestCity = pLoopCity;
							}
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		m_routeToCity = pBestCity->getIDInfo();
	}
	else
	{
		m_routeToCity.reset();
	}
}


int CvCityAI::AI_getEmphasizeYieldCount(YieldTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiEmphasizeYieldCount[eIndex];
}


bool CvCityAI::AI_isEmphasizeYield(YieldTypes eIndex)
{
	return (AI_getEmphasizeYieldCount(eIndex) > 0);
}


int CvCityAI::AI_getEmphasizeCommerceCount(CommerceTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_COMMERCE_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return (m_aiEmphasizeCommerceCount[eIndex] > 0);
}


bool CvCityAI::AI_isEmphasizeCommerce(CommerceTypes eIndex)
{
	return (AI_getEmphasizeCommerceCount(eIndex) > 0);
}


bool CvCityAI::AI_isEmphasize(EmphasizeTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumEmphasizeInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(m_pbEmphasize != NULL, "m_pbEmphasize is not expected to be equal with NULL");
	return m_pbEmphasize[eIndex];
}


void CvCityAI::AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue)
{
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumEmphasizeInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (AI_isEmphasize(eIndex) != bNewValue)
	{
		m_pbEmphasize[eIndex] = bNewValue;

		if (GC.getEmphasizeInfo(eIndex).isAvoidGrowth())
		{
			m_iEmphasizeAvoidGrowthCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeAvoidGrowthCount() >= 0);
		}

		if (GC.getEmphasizeInfo(eIndex).isGreatPeople())
		{
			m_iEmphasizeGreatPeopleCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeGreatPeopleCount() >= 0);
		}

		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			if (GC.getEmphasizeInfo(eIndex).getYieldChange(iI))
			{
				m_aiEmphasizeYieldCount[iI] += ((AI_isEmphasize(eIndex)) ? 1 : -1);
				FAssert(AI_getEmphasizeYieldCount((YieldTypes)iI) >= 0);
			}
		}

		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			if (GC.getEmphasizeInfo(eIndex).getCommerceChange(iI))
			{
				m_aiEmphasizeCommerceCount[iI] += ((AI_isEmphasize(eIndex)) ? 1 : -1);
				FAssert(AI_getEmphasizeCommerceCount((CommerceTypes)iI) >= 0);
			}
		}

		AI_assignWorkingPlots();

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}

void CvCityAI::AI_forceEmphasizeCulture(bool bNewValue)
{
	if (m_bForceEmphasizeCulture != bNewValue)
	{
		m_bForceEmphasizeCulture = bNewValue;

		m_aiEmphasizeCommerceCount[COMMERCE_CULTURE] += (bNewValue ? 1 : -1);
		FAssert(m_aiEmphasizeCommerceCount[COMMERCE_CULTURE] >= 0);
	}
}


int CvCityAI::AI_getBestBuildValue(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < NUM_CITY_PLOTS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiBestBuildValue[iIndex];
}


int CvCityAI::AI_totalBestBuildValue(CvArea* pArea)
{
	CvPlot* pLoopPlot;
	int iTotalValue;
	int iI;

	iTotalValue = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pArea)
				{
					if ((pLoopPlot->getImprovementType() == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(pLoopPlot->getImprovementType() == (GC.getDefineINT("RUINS_IMPROVEMENT")))))
					{
						iTotalValue += AI_getBestBuildValue(iI);
					}
				}
			}
		}
	}

	return iTotalValue;
}

int CvCityAI::AI_clearFeatureValue(int iIndex)
{
	CvPlot* pPlot = plotCity(getX_INLINE(), getY_INLINE(), iIndex);
	FAssert(pPlot != NULL);
	
	FeatureTypes eFeature = pPlot->getFeatureType();
	FAssert(eFeature != NO_FEATURE);
/************************************************************************************************/
/* Afforess	                  Start		 12/7/09                                                */
/************************************************************************************************/
	if (eFeature == NO_FEATURE)
		return 0;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
	CvFeatureInfo& kFeatureInfo = GC.getFeatureInfo(eFeature);
	
	int iValue = 0;
	iValue += kFeatureInfo.getYieldChange(YIELD_FOOD) * 100;
	iValue += kFeatureInfo.getYieldChange(YIELD_PRODUCTION) * 60;
	iValue += kFeatureInfo.getYieldChange(YIELD_COMMERCE) * 40;
	
	if (iValue > 0 && pPlot->isBeingWorked())
	{
		iValue *= 3;
		iValue /= 2;
	}
	if (iValue != 0)
	{
		BonusTypes eBonus = pPlot->getBonusType(getTeam());
		if (eBonus != NO_BONUS)
		{
			iValue *= 3;
			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eBonus))
				{
					iValue *= 4;
				}
			}
		}
	}
	
	int iHealthValue = 0;
	if (kFeatureInfo.getHealthPercent() != 0)
	{
		int iHealth = goodHealth() - badHealth();
		
/************************************************************************************************/
/* Afforess	                  Start		 06/17/10                                               */
/************************************************************************************************/
/*
		iHealthValue += (6 * kFeatureInfo.getHealthPercent()) / std::max(3, 1 + iHealth);
*/
	//speed up Jungle Clearing
		int iMultiplier = kFeatureInfo.getHealthPercent() > 0 ? 6 : 10;
		iHealthValue += (iMultiplier * kFeatureInfo.getHealthPercent()) / std::max(3, 1 + iHealth);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (iHealthValue > 0 && !pPlot->isBeingWorked())
		{
			iHealthValue *= 3;
			iHealthValue /= 2;
		}
	}
	iValue += iHealthValue;
	
	if (iValue > 0)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isRequiresFeature())
			{
				iValue += 500;
			}
		}
		
		if (GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0)
		{
			iValue += 400;
		}
	}
	
	return -iValue;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/25/09                                jdog5000      */
/*                                                                                              */
/* Debug                                                                                        */
/************************************************************************************************/
int CvCityAI::AI_getGoodTileCount()
{
	CvPlot* pLoopPlot;
	int iGoodTileCount = 0;
	int iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));

				bool bUseBaseValue = true;
				//If the tile has a BestBuild and is being improved, then use the BestBuild
				//determine if the tile is being improved.

				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city, changing the best build.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}
				
				if ( ((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21 )
				{
					iGoodTileCount++;
				}
			}
		}
	}

	return iGoodTileCount;
}

int CvCityAI::AI_countWorkedPoorTiles()
{
	CvPlot* pLoopPlot;
	int iWorkedPoorTileCount = 0;
	int iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this && pLoopPlot->isBeingWorked())
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));

				bool bUseBaseValue = true;
				//If the tile has a BestBuild and is being improved, then use the BestBuild
				//determine if the tile is being improved.

				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city, changing the best build.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}
				
				if ( ((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) <= 21 )
				{
					iWorkedPoorTileCount++;
				}
			}
		}
	}

	return iWorkedPoorTileCount;
}

int CvCityAI::AI_getTargetSize()
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iTargetSize = AI_getGoodTileCount();
	
/********************************************************************************/
/*	Better Evaluation							09.03.2010		Fuyu		    */
/********************************************************************************/
	int iHappyAdjust = 0;
	int iHealthAdjust = 0;
	if (getProductionBuilding() != NO_BUILDING)
	{
		iHappyAdjust += getAdditionalHappinessByBuilding(getProductionBuilding());
		iHealthAdjust += getAdditionalHealthByBuilding(getProductionBuilding());
	}
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/02/10                         jdog5000 & Fuyu      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
	//iTargetSize = std::min(iTargetSize, 2 + getPopulation() + goodHealth() - badHealth() + getEspionageHealthCounter() + std::max(0, iHealthAdjust));
	iTargetSize -= std::max(0, (iTargetSize - (1 + getPopulation() + goodHealth() - badHealth() + getEspionageHealthCounter() + std::max(0, iHealthAdjust))) / 2);
	
	if( iTargetSize < getPopulation() )
	{
		iTargetSize = std::max(iTargetSize, getPopulation() - (AI_countWorkedPoorTiles()/2));
	}
	
	// Target city size should not be perturbed by espionage, other short term effects
	iTargetSize = std::min(iTargetSize, getPopulation() + (happyLevel() - unhappyLevel() + getEspionageHappinessCounter() + std::max(0, iHappyAdjust)));
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	if (kPlayer.getAdvancedStartPoints() >= 0)
	{
		iTargetSize += 2 + kPlayer.getCurrentEra();
	}

	return iTargetSize;
}


void CvCityAI::AI_getYieldMultipliers( int &iFoodMultiplier, int &iProductionMultiplier, int &iCommerceMultiplier, int &iDesiredFoodChange )
{
	iFoodMultiplier = 100;
	iCommerceMultiplier = 100;
	iProductionMultiplier = 100;
	iDesiredFoodChange = 0;

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	CvPlot* pLoopPlot;
	int iI, iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	int iBonusFoodSurplus = 0;
	int iBonusFoodDeficit = 0;
	int iFeatureFoodSurplus = 0;
	int iHillFoodDeficit = 0;
	int iFoodTotal = GC.getYieldInfo(YIELD_FOOD).getMinCity();
	int iProductionTotal = GC.getYieldInfo(YIELD_PRODUCTION).getMinCity();

	int iWorkerCount = 0;
	int iWorkedFood = 0;
	int iWorkableFood = 0;
	int iWorkableFoodPlotCount = 0;

	int iGoodTileCount = 0;
	
	int iSpecialistCount = getSpecialistPopulation() - totalFreeSpecialists();


	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));

				bool bUseBaseValue = true;
				//If the tile has a BestBuild or is being improved, then use the BestBuild
				//determine if the tile is being improved.
				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}								

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}
				
				if (pLoopPlot->isBeingWorked())
				{
					iWorkedFood += aiFinalYields[YIELD_FOOD];					
				}
				else
				{
					if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						iWorkableFood += aiFinalYields[YIELD_FOOD];
						iWorkableFoodPlotCount++;
					}
				}
				
				if (pLoopPlot->isBeingWorked() || (((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21))
				{
					iGoodTileCount++;
					if (pLoopPlot->isBeingWorked())
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD];
					}
					else
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD] / 2;
					}
                    if (aiFinalYields[YIELD_PRODUCTION] > 1)
                    {	
                    	iProductionTotal += aiFinalYields[YIELD_PRODUCTION];
                    }
				}

				if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				{
                    int iNetFood = (aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
                    iBonusFoodSurplus += std::max(0, iNetFood);
                    iBonusFoodDeficit += std::max(0, -iNetFood);
				}

				if ((pLoopPlot->getFeatureType()) != NO_FEATURE)
				{
					iFeatureFoodSurplus += std::max(0, pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
				}

				if ((pLoopPlot->isHills()))
				{
					iHillFoodDeficit += std::max(0, GC.getFOOD_CONSUMPTION_PER_POPULATION() - pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()));
				}
			}
		}
	}


	int iBonusFoodDiff = ((iBonusFoodSurplus + iFeatureFoodSurplus) - (iBonusFoodDeficit + iHillFoodDeficit / 2));
	if (iBonusFoodDiff < 2)
	{
		iFoodMultiplier += 10 * (2 - iBonusFoodDiff);
	}

	int iHealth = goodHealth() - badHealth();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/02/10                         jdog5000 & Fuyu      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
	int iTargetSize = AI_getTargetSize();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	int iExtraFoodForGrowth = (std::max(0, iTargetSize - getPopulation()) + 3) / 4;
	if (getPopulation() < iTargetSize)
	{
		iExtraFoodForGrowth ++;
	}

	int iFoodDifference = iFoodTotal - ((iTargetSize * GC.getFOOD_CONSUMPTION_PER_POPULATION()) + iExtraFoodForGrowth);
	
	iDesiredFoodChange = -iFoodDifference + std::max(0, -iHealth);
	if (iTargetSize > getPopulation())
	{
		if (iDesiredFoodChange > 3)
		{
			iDesiredFoodChange = (iDesiredFoodChange + 3) / 2;
		}
	}

	if (iFoodDifference > 4)
	{
		iFoodMultiplier -= 8 + 4 * iFoodDifference;
	}

	if (iFoodDifference < 0)
	{
		iFoodMultiplier +=  -iFoodDifference * 4;
	}

	if (iProductionTotal < 10)
	{
	    iProductionMultiplier += (80 - 8 * iProductionTotal);
	}
	int iProductionTarget = 1 + (std::min(getPopulation(), (iTargetSize * 3) / 5));
	
	if (iProductionTotal < iProductionTarget)
	{
	    iProductionMultiplier += 8 * (iProductionTarget - iProductionTotal);
	}

	if ((iBonusFoodSurplus + iFeatureFoodSurplus > 5) && ((iBonusFoodDeficit + iHillFoodDeficit) > 5))
	{
		if ((iBonusFoodDeficit + iHillFoodDeficit) > 8)
		{
			//probably a good candidate for a wonder pump
			iProductionMultiplier += 40;
			iCommerceMultiplier += (kPlayer.AI_isFinancialTrouble()) ? 0 : -40;
		}
	}

	int iNetCommerce = 1 + kPlayer.getCommerceRate(COMMERCE_GOLD) + kPlayer.getCommerceRate(COMMERCE_RESEARCH) + std::max(0, kPlayer.getGoldPerTurn());
	int iNetExpenses = kPlayer.calculateInflatedCosts() + std::max(0, -kPlayer.getGoldPerTurn());
	int iRatio = (100 * iNetExpenses) / std::max(1, iNetCommerce);
	
	if (iRatio > 40)
	{
		iCommerceMultiplier += (33 * (iRatio - 40)) / 60;
	}
	
	// AI no longer uses emphasis except for short term boosts.
	if( isHuman() )
	{
		if (AI_isEmphasizeYield(YIELD_FOOD))
		{
			iFoodMultiplier *= 130;
			iFoodMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_PRODUCTION))
		{
			iProductionMultiplier *= 140;
			iProductionMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_COMMERCE))
		{
			iCommerceMultiplier *= 140;
			iCommerceMultiplier /= 100;
		}
	}

	int iProductionAdvantage = 100 * AI_yieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage /= kPlayer.AI_averageYieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage *= kPlayer.AI_averageYieldMultiplier(YIELD_COMMERCE);
	iProductionAdvantage /= AI_yieldMultiplier(YIELD_COMMERCE);
	
	//now we normalize the effect by # of cities
	
	int iNumCities = kPlayer.getNumCities();
	FAssert(iNumCities > 0);//superstisious?
	
	//in short in an OCC the relative multipliers should *never* make a difference
	//so this indeed equals "100" for the iNumCities == 0 case.
	iProductionAdvantage = ((iProductionAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	
	iProductionMultiplier *= iProductionAdvantage;
	iProductionMultiplier /= 100;
	
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iProductionAdvantage;
	
	int iGreatPeopleAdvantage = 100 * getTotalGreatPeopleRateModifier();
	iGreatPeopleAdvantage /= kPlayer.AI_averageGreatPeopleMultiplier();
	iGreatPeopleAdvantage = ((iGreatPeopleAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	iGreatPeopleAdvantage += 200; //gpp multipliers are larger than others so lets not go overboard
	iGreatPeopleAdvantage /= 3;
	
	//With great people we want to slightly increase food priority at the expense of commerce
	//this gracefully handles both wonder and specialist based GPP...
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iGreatPeopleAdvantage;
	iFoodMultiplier *= iGreatPeopleAdvantage;
	iFoodMultiplier /= 100;	
	
	// if leader flavor likes production, increase production, reduce commerce
	if (kPlayer.AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
	{
		iProductionMultiplier += 10;
		iCommerceMultiplier -= 10;
	}

	if (iFoodMultiplier < 100)
	{
		iFoodMultiplier = 10000 / (200 - iFoodMultiplier);
	}
	if (iProductionMultiplier < 100)
	{
		iProductionMultiplier = 10000 / (200 - iProductionMultiplier);
	}
	if (iCommerceMultiplier < 100)
	{
		iCommerceMultiplier = 10000 / (200 - iCommerceMultiplier);
	}
	
	if (angryPopulation(1) > 0)
	{
		iFoodMultiplier /= 2;
	}
}


int CvCityAI::AI_getImprovementValue( CvPlot* pPlot, ImprovementTypes eImprovement, int iFoodPriority, int iProductionPriority, int iCommercePriority, int iFoodChange, bool bOriginal )
{
	int iValue = 0;

	int aiFinalYields[NUM_YIELD_TYPES];
	int aiDiffYields[NUM_YIELD_TYPES];

	int iBestTempBuildValue = 0;
	BuildTypes eBestTempBuild = NO_BUILD;

	BonusTypes eBonus = pPlot->getBonusType(getTeam());
	BonusTypes eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(getTeam());

	bool bHasBonusImprovement = false;

	if (eNonObsoleteBonus != NO_BONUS)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))
			{
				bHasBonusImprovement = true;
			}
		}
	}

	BuildTypes eForcedBuild = NO_BUILD;
	
	{	//If a worker is already building a build, force that Build.
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;

		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getBuildType() != NO_BUILD)
			{
				if (GC.getBuildInfo(pLoopUnit->getBuildType()).getImprovement() != NO_IMPROVEMENT)
				{
					eForcedBuild = pLoopUnit->getBuildType();
					break;
				}
			}
		}
	}


	bool bIgnoreFeature = false;
	bool bValid = false;

	if (eImprovement == pPlot->getImprovementType())
	{
		bValid = true;
	}
	else
	{
		if (eForcedBuild != NO_BUILD)
		{
			if (GC.getBuildInfo(eForcedBuild).getImprovement() == eImprovement)
			{
				eBestTempBuild = eForcedBuild;
			}
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
			{
				BuildTypes eBuild = ((BuildTypes)iJ);

				if (GC.getBuildInfo(eBuild).getImprovement() == eImprovement)
				{
					if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
					{
						iValue = 10000;

						iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

						// XXX feature production???

						if (iValue > iBestTempBuildValue)
						{
							iBestTempBuildValue = iValue;
							eBestTempBuild = eBuild;
						}
					}
				}
			}
		}

		if (eBestTempBuild != NO_BUILD)
		{
			bValid = true;

			if (pPlot->getFeatureType() != NO_FEATURE)
			{
				if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
				{
					bIgnoreFeature = true;

					if (GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
					{
						if (eNonObsoleteBonus == NO_BONUS)
						{
							if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
							{
								bValid = false;
							}
							else if (healthRate() < 0 && GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() > 0)
							{
								bValid = false;
							}
							else if (GET_PLAYER(getOwnerINLINE()).getFeatureHappiness(pPlot->getFeatureType()) > 0)
							{
								bValid = false;
							}
						}
					}
				}
			}
		}
	}

	if (bValid)
	{
		ImprovementTypes eFinalImprovement = finalImprovementUpgrade(eImprovement);

		if (eFinalImprovement == NO_IMPROVEMENT)
		{
			eFinalImprovement = eImprovement;
		}

		iValue = 0;
		
		if (eBonus != NO_BONUS)
		{
			if (eNonObsoleteBonus != NO_BONUS)
			{
				if (GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
				{
					iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
					iValue += 200;
					/*if (eBestBuild != NO_BUILD)
					{
						if ((GC.getBuildInfo(eBestBuild).getImprovement() == NO_IMPROVEMENT) || (!GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
						{
							//Always prefer improvements which connect bonuses.
							eBestBuild = NO_BUILD;
							iBestValue = 0;
						}
					}*/
				}
				else
				{
					/*if (eBestBuild != NO_BUILD)
					{
						if ((GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT) && (GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
						{
							iValue -= 1000;
						}
					}*/
				}
			}
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
			{
				if (GC.getImprovementInfo(eFinalImprovement).getImprovementBonusDiscoverRand(iJ) > 0)
				{
					iValue++;
				}
			}
		}
		
		if (iValue >= 0)
		{

            iValue *= 2;
			for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				if( false && bOriginal )
				{
					aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
					if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
					{
						aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);							
					}
					aiDiffYields[iJ] = (aiFinalYields[iJ] - (2 * pPlot->getYield(((YieldTypes)iJ))));
				}
				else
				{
					aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
					if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
					{
						aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);							
					}

					int iCurYield = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), false));

					ImprovementTypes eCurImprovement = pPlot->getImprovementType();
					if( eCurImprovement != NO_IMPROVEMENT )
					{
						ImprovementTypes eCurFinalImprovement = finalImprovementUpgrade(eCurImprovement);
						if (eCurFinalImprovement == NO_IMPROVEMENT)
						{
							eCurFinalImprovement = eCurImprovement;
						}
						iCurYield += (pPlot->calculateImprovementYieldChange(eCurFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						iCurYield += (pPlot->calculateImprovementYieldChange(eCurImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
					}

					aiDiffYields[iJ] = (aiFinalYields[iJ] - iCurYield);
				}
			}

			iValue += (aiDiffYields[YIELD_FOOD] * ((100 * iFoodPriority) / 100));
			iValue += (aiDiffYields[YIELD_PRODUCTION] * ((60 * iProductionPriority) / 100));
			iValue += (aiDiffYields[YIELD_COMMERCE] * ((40 * iCommercePriority) / 100));

			iValue /= 2;

            for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				aiFinalYields[iJ] /= 2;
				aiDiffYields[iJ] /= 2;
			}
			
			if (iValue > 0)
			{
				// this is mainly to make it improve better tiles first
				//flood plain > grassland > plain > tundra
				iValue += (aiFinalYields[YIELD_FOOD] * 10);
				iValue += (aiFinalYields[YIELD_PRODUCTION] * 6);
				iValue += (aiFinalYields[YIELD_COMMERCE] * 4);

				if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
				{
					//this is a food yielding tile
					if (iFoodPriority > 100)
					{
						iValue *= 100 + iFoodPriority;
						iValue /= 200;							
					}
					if (iFoodChange > 0)
					{
						iValue += (10 * (1 + aiDiffYields[YIELD_FOOD]) * (1 + aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION()) * iFoodChange * iFoodPriority) / 100;
					}
					if (iCommercePriority > 100)
					{
						iValue *= 100 + (((iCommercePriority - 100) * aiDiffYields[YIELD_COMMERCE]) / 2);
						iValue /= 100;
					}
				}
				else if (aiFinalYields[YIELD_FOOD] < GC.getFOOD_CONSUMPTION_PER_POPULATION())
				{
					if ((aiDiffYields[YIELD_PRODUCTION] > 0) && (aiFinalYields[YIELD_FOOD]+aiFinalYields[YIELD_PRODUCTION] > 3))
					{
						if (iFoodPriority < 100 || GET_PLAYER(getOwnerINLINE()).getCurrentEra() < 2)
						{
							//value booster for mines on hills
							iValue *= (100 + 25 * aiDiffYields[YIELD_PRODUCTION]);
							iValue /= 100;
						}
					}
					if (iFoodChange < 0)
					{
						iValue *= 4 - iFoodChange;
						iValue /= 3 + aiFinalYields[YIELD_FOOD];
					}
				}

				if ((iFoodPriority < 100) && (iProductionPriority > 100))
				{
					if( bOriginal )
					{
						iValue *= (200 + iProductionPriority);
					}
					else
					{
						iValue *= (200 + ((iProductionPriority - 100)*aiFinalYields[YIELD_PRODUCTION]));
					}
					iValue /= 200;
				}
				if (eBonus == NO_BONUS)
				{
					if (iFoodChange > 0)
					{
						//We want more food.
						iValue *= 2 + std::max(0, aiDiffYields[YIELD_FOOD]);
						iValue /= 2 * (1 + std::max(0, -aiDiffYields[YIELD_FOOD]));
					}
//							else if (iFoodChange < 0)
//							{
//								//We want to soak up food.
//								iValue *= 8;
//								iValue /= 8 + std::max(0, aiDiffYields[YIELD_FOOD]);
//							}
				}
			}
			

			/*if (bEmphasizeIrrigation && GC.getImprovementInfo(eFinalImprovement).isCarriesIrrigation())
			{
				iValue += 500;
			}*/

			if (getImprovementFreeSpecialists(eFinalImprovement) > 0)
			{
				iValue += 2000;
			}
			
			int iHappiness = GC.getImprovementInfo(eFinalImprovement).getHappiness();
			if ((iHappiness != 0) && !(GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0))
			{
				//int iHappyLevel = iHappyAdjust + (happyLevel() - unhappyLevel(0));
				int iHappyLevel = 0 + (happyLevel() - unhappyLevel(0));
				if (eImprovement == pPlot->getImprovementType())
				{
					iHappyLevel -= iHappiness;
				}
				int iHealthLevel = (goodHealth() - badHealth(false, 0));
				
				int iHappyValue = 0;
				if (iHappyLevel <= 0)
				{
					iHappyValue += 400;
				}
				bool bCanGrow = true;// (getYieldRate(YIELD_FOOD) > foodConsumption());
				
				if (iHappyLevel <= iHealthLevel)
				{
					iHappyValue += 200 * std::max(0, (bCanGrow ? std::min(6, 2 + iHealthLevel - iHappyLevel) : 0) - iHappyLevel);
				}
				else
				{
					iHappyValue += 200 * std::max(0, (bCanGrow ? 1 : 0) - iHappyLevel);
				}
				if (!pPlot->isBeingWorked())
				{
					iHappyValue *= 4;
					iHappyValue /= 3;
				}
				iHappyValue += std::max(0, (pPlot->getCityRadiusCount() - 1)) * ((iHappyValue > 0) ? iHappyLevel / 2 : 200);
				iValue += iHappyValue * iHappiness;
			}

			if (!isHuman())
			{
				iValue *= std::max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getImprovementWeightModifier(eFinalImprovement) + 200));
				iValue /= 200;
			}

			if (pPlot->getImprovementType() == NO_IMPROVEMENT)
			{
				if (pPlot->isBeingWorked())
				{
					iValue *= 5;
					iValue /= 4;
				}

				if (eBestTempBuild != NO_BUILD)
				{
					if (pPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							CvCity* pCity = NULL;
							iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity) * 2;
							FAssert(pCity == this);

							//iValue += iClearFeatureValue;
						}
					}
				}
			}
			else
			{
				// cottage/villages (don't want to chop them up if turns have been invested)
				ImprovementTypes eImprovementDowngrade = (ImprovementTypes)GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage();
				while (eImprovementDowngrade != NO_IMPROVEMENT)
				{
					CvImprovementInfo& kImprovementDowngrade = GC.getImprovementInfo(eImprovementDowngrade);
					iValue -= kImprovementDowngrade.getUpgradeTime() * 8;
					eImprovementDowngrade = (ImprovementTypes)kImprovementDowngrade.getImprovementPillage();
				}

				if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
				{
					iValue -= (GC.getImprovementInfo(pPlot->getImprovementType()).getUpgradeTime() * 8 * (pPlot->getUpgradeProgress())) / std::max(1, GC.getGameINLINE().getImprovementUpgradeTime(pPlot->getImprovementType()));
				}

				if (eNonObsoleteBonus == NO_BONUS)
				{
					if (isWorkingPlot(pPlot))
					{
						if (((iFoodPriority < 100) && (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())) || (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage() != NO_IMPROVEMENT))
						{
							iValue -= 70;
							iValue *= 2;
							iValue /= 3;
						}
					}
				}
				
				if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
				{
					iValue /= 4;	//Greatly prefer builds which are legal.
				}
			}
		}
	}

	return iValue;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	
BuildTypes CvCityAI::AI_getBestBuild(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < NUM_CITY_PLOTS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aeBestBuild[iIndex];
}


int CvCityAI::AI_countBestBuilds(CvArea* pArea)
{
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pArea)
				{
					if (AI_getBestBuild(iI) != NO_BUILD)
					{
						iCount++;
					}
				}
			}
		}
	}

	return iCount;
}


// Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_updateBestBuild()
{
	PROFILE_FUNC();
	
	
	CvPlot* pLoopPlot;
	int iI, iJ;
    int aiFinalYields[NUM_YIELD_TYPES];

	int iBonusFoodSurplus = 0;
	int iBonusFoodDeficit = 0;
	int iFeatureFoodSurplus = 0;
	int iHillFoodDeficit = 0;
	int iFoodTotal = GC.getYieldInfo(YIELD_FOOD).getMinCity();
	int iProductionTotal = GC.getYieldInfo(YIELD_PRODUCTION).getMinCity();
	int iFoodMultiplier = 100;
	int iCommerceMultiplier = 100;
	int iProductionMultiplier = 100;
	int iWorkerCount = 0;
	
	int iWorkedFood = 0;
	int iWorkableFood = 0;
	int iWorkableFoodPlotCount = 0;
	
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	bool bChop = false;

	int iGoodTileCount = 0;
	
	int iSpecialistCount = getSpecialistPopulation() - totalFreeSpecialists();

	int iHappyAdjust = 0;
	int iHealthAdjust = 0;
	
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));
				iWorkerCount += iCount;

				bool bUseBaseValue = true;
				//If the tile has a BestBuild and is being improved, then use the BestBuild
				//determine if the tile is being improved.
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/25/09                                jdog5000      */
/*                                                                                              */
/* Worker AI, City AI                                                                           */
/************************************************************************************************/
				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city, changing the best build.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}
							
							iHappyAdjust += GC.getImprovementInfo(eImprovement).getHappiness();
							if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
							{
								iHappyAdjust -= GC.getImprovementInfo(pLoopPlot->getImprovementType()).getHappiness();
							}
								

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}
				
				if (pLoopPlot->isBeingWorked())
				{
					iWorkedFood += aiFinalYields[YIELD_FOOD];					
				}
				else
				{
					if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						iWorkableFood += aiFinalYields[YIELD_FOOD];
						iWorkableFoodPlotCount++;
					}
				}
				
				if (pLoopPlot->isBeingWorked() || (((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21))
				{
					iGoodTileCount++;
					if (pLoopPlot->isBeingWorked())
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD];
					}
					else
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD] / 2;
					}
                    if (aiFinalYields[YIELD_PRODUCTION] > 1)
                    {	
                    	iProductionTotal += aiFinalYields[YIELD_PRODUCTION];
                    }
				}

				if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				{
                    int iNetFood = (aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
                    iBonusFoodSurplus += std::max(0, iNetFood);
                    iBonusFoodDeficit += std::max(0, -iNetFood);
				}

				if ((pLoopPlot->getFeatureType()) != NO_FEATURE)
				{
					iFeatureFoodSurplus += std::max(0, pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
				}

				if ((pLoopPlot->isHills()))
				{
					iHillFoodDeficit += std::max(0, GC.getFOOD_CONSUMPTION_PER_POPULATION() - pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()));
				}
			}
		}
	}

    if (getProductionBuilding() != NO_BUILDING)
    {
/********************************************************************************/
/*	Better Evaluation							09.03.2010		Fuyu		    */
/********************************************************************************/
/* original code
    	iHappyAdjust += getBuildingHappiness(getProductionBuilding());
    	iHealthAdjust += getBuildingHealth(getProductionBuilding());    	
*/
    	iHappyAdjust += getAdditionalHappinessByBuilding(getProductionBuilding());
    	iHealthAdjust += getAdditionalHealthByBuilding(getProductionBuilding());
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/
    }

	//XXX rewrite this to fix too many farms issue
	int iSpecialistAdjustment = 0;
	if (iWorkableFoodPlotCount > 0)
	{
		//Calculate approximately how much food the assigned specialists could work
		iSpecialistAdjustment = (std::min(iSpecialistCount, iWorkableFoodPlotCount) * iWorkableFood) / iWorkableFoodPlotCount;
	}
	iFoodTotal += iSpecialistAdjustment;
	
	int iBonusFoodDiff = ((iBonusFoodSurplus + iFeatureFoodSurplus) - (iBonusFoodDeficit + iHillFoodDeficit / 2));

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/02/10                         jdog5000 & Fuyu      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
	int iHealth = goodHealth() - badHealth();
	int iTargetSize = iGoodTileCount;

	//iTargetSize = std::min(iTargetSize, 2 + getPopulation() + goodHealth() - badHealth() + getEspionageHealthCounter() + std::max(0, iHealthAdjust));
	iTargetSize -= std::max(0, (iTargetSize - (1 + getPopulation() + goodHealth() - badHealth() + getEspionageHealthCounter() + std::max(0, iHealthAdjust))) / 2);
	
	if( iTargetSize < getPopulation() )
	{
		iTargetSize = std::max(iTargetSize, getPopulation() - (AI_countWorkedPoorTiles()/2));
	}
	
	// Target city size should not be perturbed by espionage, other short term effects
	iTargetSize = std::min(iTargetSize, getPopulation() + (happyLevel() - unhappyLevel() + getEspionageHappinessCounter() + std::max(0, iHappyAdjust)));
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/02/09                                jdog5000      */
/*                                                                                              */
/* Worker AI                                                                                    */
/************************************************************************************************/
/* original bts code
	if (GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0)
	{
		iTargetSize += 2 + GET_PLAYER(getOwnerINLINE()).getCurrentEra() / 2;
	}
	
	if (kPlayer.getAdvancedStartPoints() >= 0)
	{
		iTargetSize += kPlayer.getCurrentEra() / 2;
	}
*/
	// WTF code duplication
	if (kPlayer.getAdvancedStartPoints() >= 0)
	{
		iTargetSize += 2 + kPlayer.getCurrentEra();
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	if (iBonusFoodDiff < 2)
	{
		iFoodMultiplier += 10 * (2 - iBonusFoodDiff);
	}
	
	int iExtraFoodForGrowth = (std::max(0, iTargetSize - getPopulation()) + 3) / 4;
	if (getPopulation() < iTargetSize)
	{
		iExtraFoodForGrowth ++;
	}

	int iFoodDifference = iFoodTotal - ((iTargetSize * GC.getFOOD_CONSUMPTION_PER_POPULATION()) + iExtraFoodForGrowth);
	
	int iDesiredFoodChange = -iFoodDifference + std::max(0, -iHealth);
	if (iTargetSize > getPopulation())
	{
		if (iDesiredFoodChange > 3)
		{
			iDesiredFoodChange = (iDesiredFoodChange + 3) / 2;
		}
	}

	if (iFoodDifference < 0)
	{
		iFoodMultiplier +=  -iFoodDifference * 4;
	}
	
	if (iFoodDifference > 4)
	{
		iFoodMultiplier -= 8 + 4 * iFoodDifference;
	}

	if (iProductionTotal < 10)
	{
	    iProductionMultiplier += (80 - 8 * iProductionTotal);
	}
	int iProductionTarget = 1 + (std::min(getPopulation(), (iTargetSize * 3) / 5));
	
	if (iProductionTotal < iProductionTarget)
	{
	    iProductionMultiplier += 8 * (iProductionTarget - iProductionTotal);
	}

	if ((iBonusFoodSurplus + iFeatureFoodSurplus > 5) && ((iBonusFoodDeficit + iHillFoodDeficit) > 5))
	{
		if ((iBonusFoodDeficit + iHillFoodDeficit) > 8)
		{
			//probably a good candidate for a wonder pump
			iProductionMultiplier += 40;
			iCommerceMultiplier += (kPlayer.AI_isFinancialTrouble()) ? 0 : -40;
		}
	}
	
	
	int iNetCommerce = 1 + kPlayer.getCommerceRate(COMMERCE_GOLD) + kPlayer.getCommerceRate(COMMERCE_RESEARCH) + std::max(0, kPlayer.getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/11/09                       jdog5000 & DanF5771    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original BTS code
	int iNetExpenses = kPlayer.calculateInflatedCosts() + std::min(0, kPlayer.getGoldPerTurn());
*/
	int iNetExpenses = kPlayer.calculateInflatedCosts() + std::max(0, -kPlayer.getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	int iRatio = (100 * iNetExpenses) / std::max(1, iNetCommerce);
	
	if (iRatio > 40)
	{
		iCommerceMultiplier += (33 * (iRatio - 40)) / 60;
	}
	
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/06/09                                jdog5000      */
/*                                                                                              */
/* Worker AI                                                                                    */
/************************************************************************************************/
	// AI no longer uses emphasis except for short term boosts.
	if( isHuman() )
	{
		if (AI_isEmphasizeYield(YIELD_FOOD))
		{
			iFoodMultiplier *= 130;
			iFoodMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_PRODUCTION))
		{
			iProductionMultiplier *= 140;
			iProductionMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_COMMERCE))
		{
			iCommerceMultiplier *= 140;
			iCommerceMultiplier /= 100;
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	
	int iProductionAdvantage = 100 * AI_yieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage /= kPlayer.AI_averageYieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage *= kPlayer.AI_averageYieldMultiplier(YIELD_COMMERCE);
	iProductionAdvantage /= AI_yieldMultiplier(YIELD_COMMERCE);
	
	//now we normalize the effect by # of cities
	
	int iNumCities = kPlayer.getNumCities();
	FAssert(iNumCities > 0);//superstisious?
	
	//in short in an OCC the relative multipliers should *never* make a difference
	//so this indeed equals "100" for the iNumCities == 0 case.
	iProductionAdvantage = ((iProductionAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	
	iProductionMultiplier *= iProductionAdvantage;
	iProductionMultiplier /= 100;
	
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iProductionAdvantage;
	
	int iGreatPeopleAdvantage = 100 * getTotalGreatPeopleRateModifier();
	iGreatPeopleAdvantage /= kPlayer.AI_averageGreatPeopleMultiplier();
	iGreatPeopleAdvantage = ((iGreatPeopleAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	iGreatPeopleAdvantage += 200; //gpp multipliers are larger than others so lets not go overboard
	iGreatPeopleAdvantage /= 3;
	
	//With great people we want to slightly increase food priority at the expense of commerce
	//this gracefully handles both wonder and specialist based GPP...
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iGreatPeopleAdvantage;
	iFoodMultiplier *= iGreatPeopleAdvantage;
	iFoodMultiplier /= 100;	
	
	// if leader flavor likes production, increase production, reduce commerce
	if (kPlayer.AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
	{
		iProductionMultiplier += 10;
		iCommerceMultiplier -= 10;
	}
	
	if (iFoodMultiplier < 100)
	{
		iFoodMultiplier = 10000 / (200 - iFoodMultiplier);
	}
	if (iProductionMultiplier < 100)
	{
		iProductionMultiplier = 10000 / (200 - iProductionMultiplier);
	}
	if (iCommerceMultiplier < 100)
	{
		iCommerceMultiplier = 10000 / (200 - iCommerceMultiplier);
	}
	
	if (angryPopulation(1) > 0)
	{
		iFoodMultiplier /= 2;
	}

    if (!bChop)
	{
		ProjectTypes eProductionProject = getProductionProject();
		bChop = (eProductionProject != NO_PROJECT && AI_projectValue(eProductionProject) > 0);
	}
	if (!bChop)
	{
		BuildingTypes eProductionBuilding = getProductionBuilding();
		bChop = (eProductionBuilding != NO_BUILDING && isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())));
	}
	if (!bChop)
	{
		UnitTypes eProductionUnit = getProductionUnit();
		bChop = (eProductionUnit != NO_UNIT && GC.getUnitInfo(eProductionUnit).isFoodProduction());
	}
	if (!bChop)
	{
        bChop = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
    }
    
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aiBestBuildValue[iI] = 0;
		m_aeBestBuild[iI] = NO_BUILD;

		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/16/10                                jdog5000      */
/*                                                                                              */
/* City AI, Worker AI                                                                           */
/************************************************************************************************/
				int iLastBestBuildValue = m_aiBestBuildValue[iI];
				BuildTypes eLastBestBuildType = m_aeBestBuild[iI];

				AI_bestPlotBuild(pLoopPlot, &(m_aiBestBuildValue[iI]), &(m_aeBestBuild[iI]), iFoodMultiplier, iProductionMultiplier, iCommerceMultiplier, bChop, iHappyAdjust, iHealthAdjust, iDesiredFoodChange);
				m_aiBestBuildValue[iI] *= 4;
				m_aiBestBuildValue[iI] += 3 + iWorkerCount;  // to round up
				m_aiBestBuildValue[iI] /= (4 + iWorkerCount);
				
				if (m_aiBestBuildValue[iI] > 0)
				{
					FAssert(m_aeBestBuild[iI] != NO_BUILD);
				}
				if (m_aeBestBuild[iI] != NO_BUILD)
				{
					FAssert(m_aiBestBuildValue[iI] > 0);
				}

				if( eLastBestBuildType != NO_BUILD )
				{
					if( eLastBestBuildType != m_aeBestBuild[iI] )
					{
						if( iWorkerCount > 0 )
						{
							// BBAI TODO: Check workers are building the new thing

							if( gCityLogLevel >= 2 )
							{
								logBBAI( "      City %S switches best build on plot %d, %d from %S (%d) to %S (%d) with worker count %d",getName().GetCString(),pLoopPlot->getX(),pLoopPlot->getY(),GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eLastBestBuildType).getImprovement()).getDescription(),iLastBestBuildValue,GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(m_aeBestBuild[iI]).getImprovement()).getDescription(),m_aiBestBuildValue[iI],iWorkerCount);
							}
						}
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			}
		}
	}
	
	
	{	//new experimental yieldValue calcuation
		short aiYields[NUM_YIELD_TYPES];
		int iBestPlot = -1;
		int iBestPlotValue = -1;
		int iValue;
		
		int iBestUnworkedPlotValue = 0;
		
		int aiValues[NUM_CITY_PLOTS];
		
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
				{
					if (m_aeBestBuild[iI] != NO_BUILD)
					{
						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiYields[iJ] = pLoopPlot->getYieldWithBuild(m_aeBestBuild[iI], (YieldTypes)iJ, true);
						}
						
						iValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						aiValues[iI] = iValue;
						if ((iValue > 0) && (pLoopPlot->getRouteType() != NO_ROUTE))
						{
							iValue++;
						}
						//FAssert(iValue > 0);
						
						iValue = std::max(0, iValue);
						
						m_aiBestBuildValue[iI] *= iValue + 100;
						m_aiBestBuildValue[iI] /= 100;
						
						if (iValue > iBestPlotValue)
						{
							iBestPlot = iI;
							iBestPlotValue = iValue;							
						}
					}
					if (!pLoopPlot->isBeingWorked())
					{
						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						}
						
						iValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);					
						
						iBestUnworkedPlotValue = std::max(iBestUnworkedPlotValue, iValue);
					}
				}
			}
		}
		if (iBestPlot != -1)
		{
			m_aiBestBuildValue[iBestPlot] *= 2;
		}
		
		//Prune plots which are sub-par.
		if (iBestUnworkedPlotValue > 0)
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				if (iI != CITY_HOME_PLOT)
				{
					pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

					if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
					{
						if (m_aeBestBuild[iI] != NO_BUILD) 
						{
							if (!pLoopPlot->isBeingWorked() && (pLoopPlot->getImprovementType() == NO_IMPROVEMENT))
							{
								if (GC.getBuildInfo(m_aeBestBuild[iI]).getImprovement() != NO_IMPROVEMENT)
								{
									if ((aiValues[iI] <= iBestUnworkedPlotValue) && (aiValues[iI] < 500))
									{
										m_aiBestBuildValue[iI] = 1;
									}
								}
							}
							else if ((pLoopPlot->getImprovementType() != NO_IMPROVEMENT) && (GC.getBuildInfo(m_aeBestBuild[iI]).getImprovement() != NO_IMPROVEMENT))
							{
								for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
								{
									aiYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
								}
								
								iValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
								if (iValue > aiValues[iI])
								{
									m_aiBestBuildValue[iI] = 1;
								}
							}
						}
					}
				}
			}
		}
	}
}

// Protected Functions...

// Better drafting strategy by Blake - thank you!
void CvCityAI::AI_doDraft(bool bForce)
{
	PROFILE_FUNC();

	FAssert(!isHuman());
	if (isBarbarian())
	{
		return;
	}

	if (canConscript())
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/12/09                                jdog5000      */
/*                                                                                              */
/* City AI, War Strategy AI                                                                     */
/************************************************************************************************/
	    if (GC.getGameINLINE().AI_combatValue(getConscriptUnit()) > 33)
        {
			if (bForce)
			{
				conscript();
				return;
        	}
			bool bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
            bool bDanger = (!AI_isDefended() && AI_isDanger());

			// Don't go broke from drafting
			if( !bDanger && GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble() )
			{
				return;
			}

			// Don't shrink cities too much
            int iConscriptPop = getConscriptPopulation();
			if ( !bDanger && (3 * (getPopulation() - iConscriptPop) < getHighestPopulation() * 2) )
            {
				return;
            }

			// Large cities want a little spare happiness
			int iHappyDiff = GC.getDefineINT("CONSCRIPT_POP_ANGER") - iConscriptPop + getPopulation()/10;

            if (bLandWar && (0 == angryPopulation(iHappyDiff)))
            {
                bool bWait = true;

				if( bWait && GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
				{
					// Full out defensive
					if( bDanger || (getPopulation() >= std::max(5, getHighestPopulation() - 1)) )
					{
						bWait = false;
					}
					else if( (AI_countWorkedPoorTiles() + std::max(0,(visiblePopulation() - AI_countGoodSpecialists(true)))) >= 1 )
					{
						bWait = false;
					}
				}
				
				if( bWait && bDanger )
				{
					// If city might be captured, don't hold back
					int iOurDefense = GET_TEAM(getTeam()).AI_getOurPlotStrength(plot(),0,true,false,true);
					int iEnemyOffense = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(),2,false,false);

					if( (iOurDefense == 0) || (3*iEnemyOffense > 2*iOurDefense) )
					{
						bWait = false;
					}
				}

				if( bWait )
				{
					// Non-critical, only burn population if population is not worth much
					if ((getConscriptAngerTimer() == 0) && ((AI_countWorkedPoorTiles() + std::max(0,(visiblePopulation() - AI_countGoodSpecialists(true)))) >= iConscriptPop))
					{
						if( (getPopulation() >= std::max(5, getHighestPopulation() - 1)) )
						{
							bWait = false;
						}
					}
				}

				if( !bWait && gCityLogLevel >= 2 )
				{
					logBBAI("      City %S (size %d, highest %d) chooses to conscript with danger: %d, land war: %d, poor tiles: %d, bad specialists: %d", getName().GetCString(), getPopulation(), getHighestPopulation(), bDanger, bLandWar, AI_countWorkedPoorTiles(), std::max(0,(visiblePopulation() - AI_countGoodSpecialists(true))) );
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

                if (!bWait)
                {
                    conscript();
                }
            }
        }
	}
}

// Better pop-rushing strategy by Blake - thank you!
void CvCityAI::AI_doHurry(bool bForce)
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	UnitTypes eProductionUnit;
	UnitAITypes eProductionUnitAI;
	BuildingTypes eProductionBuilding;
	int iHurryAngerLength;
	int iHurryPopulation;
	int iMinTurns;
	bool bDanger;
	bool bWait;
	bool bEssential;
	bool bGrowth;
	int iI, iJ;

	FAssert(!isHuman() || isProductionAutomated());
	
	if (isBarbarian())
	{
		return;
	}

	if ((getProduction() == 0) && !bForce)
	{
		return;
	}

	pWaterArea = waterArea();

	eProductionUnit = getProductionUnit();
	eProductionUnitAI = getProductionUnitAI();
	eProductionBuilding = getProductionBuilding();

	bDanger = AI_isDanger();

	for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		if (canHurry((HurryTypes)iI))
		{
			if (bForce)
			{
				hurry((HurryTypes)iI);
				break;
			}
			iHurryAngerLength = hurryAngerLength((HurryTypes)iI);
			iHurryPopulation = hurryPopulation((HurryTypes)iI);

			iMinTurns = MAX_INT;
			bEssential = false;
			bGrowth = false;

			// Whip to eliminate unhappiness - thank you Blake!
			if (getProduction() > 0)
			{
				if (AI_getHappyFromHurry((HurryTypes)iI) > 0)
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
					if( gCityLogLevel >= 2 )
					{
						logBBAI("      City %S hurry to remove unhappiness", getName().GetCString() );
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					hurry((HurryTypes)iI);
					break;
				}
			}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/07/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
			// Rush defenses when in big trouble
			if ( (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) && GET_TEAM(getTeam()).AI_getEnemyPowerPercent(true) > 150 )
			{
				if( eProductionUnit != NO_UNIT && GC.getGameINLINE().AI_combatValue(eProductionUnit) > 33 && getProduction() > 0 )
				{
					if( (iHurryPopulation > 0) && (iHurryAngerLength == 0 || getHurryAngerTimer() < 2) && (iHurryPopulation < 3 && iHurryPopulation < getPopulation()/3))
					{
						bool bWait = true;

						if( GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
						{
							bWait = false;
						}
						else if( (3*(getPopulation() - iHurryPopulation)) < getHighestPopulation()*2 )
						{
							bWait = true;
						}
						else if( GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble() )
						{
							bWait = true;
						}
						else
						{
							for( int iJ = 0; iJ < MAX_CIV_TEAMS; iJ++ )
							{
								if( GET_TEAM((TeamTypes)iJ).isAlive() && !GET_TEAM((TeamTypes)iJ).isMinorCiv() )
								{
									if( GET_TEAM(getTeam()).isAtWar((TeamTypes)iJ) && GET_TEAM(getTeam()).AI_getAtWarCounter((TeamTypes)iJ) < 10 )
									{
										bWait = false;
										break;
									}
								}
							}
						}

						if( !bWait )
						{
							if( gCityLogLevel >= 2 )
							{
								logBBAI("      City %S hurry pop at %d to rush defenses for recent attack", getName().GetCString(), iHurryPopulation );
							}
							hurry((HurryTypes)iI);
							break;
						}
					}
					else
					{
						if( !(GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble()) )
						{
							int iHurryGold = hurryGold((HurryTypes)iI);
							if( iHurryGold > 0 && iHurryAngerLength == 0 )
							{
								bool bDanger = AI_isDanger();
								bool bWait = true;

								if( GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
								{
									if( (bDanger ? 5 : 8)*iHurryGold < GET_PLAYER(getOwnerINLINE()).getGold() )
									{
										bWait = false;
									}
								}
								else
								{
									if( (bDanger ? 8 : 12)*iHurryGold < GET_PLAYER(getOwnerINLINE()).getGold() )
									{
										bWait = false;
									}
								}

								if( !bWait )
								{
									if( gCityLogLevel >= 2 )
									{
										logBBAI("      City %S hurry gold at %d to rush defenses for recent attack", getName().GetCString(), iHurryGold );
									}
									hurry((HurryTypes)iI);
									break;
								}
							}
						}
					}
				}
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

			if ((iHurryAngerLength == 0) && (iHurryPopulation == 0))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_avoidScience())
				{
					if (GET_PLAYER(getOwnerINLINE()).getGold() > GET_PLAYER(getOwnerINLINE()).AI_goldTarget())
					{
						iMinTurns = std::min(iMinTurns, 10);
					}
				}

				if (eProductionBuilding != NO_BUILDING)
				{
					int iValuePerTurn = AI_buildingValueThreshold(eProductionBuilding, BUILDINGFOCUS_GOLD | BUILDINGFOCUS_MAINTENANCE | BUILDINGFOCUS_PRODUCTION);
					
					iValuePerTurn /= 3;
					
					if (iValuePerTurn > 0)
					{
						int iHurryGold = hurryGold((HurryTypes)iI);
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/06/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
						if ((iHurryGold / iValuePerTurn) < getProductionTurnsLeft(eProductionBuilding, 1))
*/
						if ( (iHurryGold > 0) && ((iHurryGold / iValuePerTurn) < getProductionTurnsLeft(eProductionBuilding, 1)) )
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
						{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/06/09                                jdog5000      */
/*                                                                                              */
/* Gold AI                                                                                      */
/************************************************************************************************/
/* original bts code
							if (iHurryGold < (GET_PLAYER(getOwnerINLINE()).getGold() / 3))
*/
							int iGoldThreshold = GET_PLAYER(getOwnerINLINE()).getGold();
							iGoldThreshold -= (GET_PLAYER(getOwnerINLINE()).AI_goldToUpgradeAllUnits() / ((GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0) ? 1 : 3));
							iGoldThreshold /= 3;
							if (iHurryGold < iGoldThreshold)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		
							{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
								if( gCityLogLevel >= 2 )
								{
									logBBAI("      City %S hurry gold at %d < threshold %d", getName().GetCString(), iHurryGold, iGoldThreshold );
								}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
								hurry((HurryTypes)iI);
								return;
							}								
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())))
				{
					iMinTurns = std::min(iMinTurns, 10);
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getDefenseModifier() > 0)
				{
					if (bDanger)
					{
						iMinTurns = std::min(iMinTurns, 3);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getBombardDefenseModifier() > 0)
				{
					if (bDanger)
					{
						iMinTurns = std::min(iMinTurns, 3);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getYieldModifier(YIELD_PRODUCTION) > 0)
				{
					if (getBaseYieldRate(YIELD_PRODUCTION) >= 6)
					{
						iMinTurns = std::min(iMinTurns, 10);
						bGrowth = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if ((GC.getBuildingInfo(eProductionBuilding).getCommerceChange(COMMERCE_CULTURE) > 0) ||
						(GC.getBuildingInfo(eProductionBuilding).getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0))
				{
					if ((getCommerceRateTimes100(COMMERCE_CULTURE) == 0) || (plot()->calculateCulturePercent(getOwnerINLINE()) < 40))
					{
						iMinTurns = std::min(iMinTurns, 10);
						if (getCommerceRateTimes100(COMMERCE_CULTURE) == 0)
						{
						    bEssential = true;
						    iMinTurns = std::min(iMinTurns, 5);
						    if (AI_countNumBonuses(NO_BONUS, false, true, 2, true, true) > 0)
						    {
						    	bGrowth = true;						    	
						    }
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getHappiness() > 0)
				{
					if (angryPopulation() > 0)
					{
						iMinTurns = std::min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getHealth() > 0)
				{
					if (healthRate() < 0)
					{
						iMinTurns = std::min(iMinTurns, 10);
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getSeaPlotYieldChange(YIELD_FOOD) > 0 || GC.getBuildingInfo(eProductionBuilding).getRiverPlotYieldChange(YIELD_FOOD) > 0)
				{
					
					iMinTurns = std::min(iMinTurns, 10);
					
					if (AI_buildingSpecialYieldChangeValue(eProductionBuilding, YIELD_FOOD) > (getPopulation() * 2))
					{
						bEssential = true;
						bGrowth = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getFreeExperience() > 0)
				{
					if (bDanger)
					{
						iMinTurns = std::min(iMinTurns, 3);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getMaintenanceModifier() < 0)
				{
					if (getMaintenance() >= 10)
					{
						iMinTurns = std::min(iMinTurns, 10);
						bEssential = true;
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
				{
					if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > 0)
					{
						for (iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
						{
							if (GC.getBuildingInfo(eProductionBuilding).getSpecialistCount(iJ) > 0)
							{
								iMinTurns = std::min(iMinTurns, 10);
								break;
							}
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getCommerceModifier(COMMERCE_GOLD) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
					{
						if (getBaseCommerceRate(COMMERCE_GOLD) >= 16)
						{
							iMinTurns = std::min(iMinTurns, 10);
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getCommerceModifier(COMMERCE_RESEARCH) > 0)
				{
					if (!(GET_PLAYER(getOwnerINLINE()).AI_avoidScience()))
					{
						if (getBaseCommerceRate(COMMERCE_RESEARCH) >= 16)
						{
							iMinTurns = std::min(iMinTurns, 10);
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eProductionBuilding).getFoodKept() > 0)
				{
					iMinTurns = std::min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
				}
			}

			if (eProductionUnit != NO_UNIT)
			{
				if (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						if (bDanger)
						{
							iMinTurns = std::min(iMinTurns, 3);
							bEssential = true;
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_CITY_DEFENSE)
			{
				if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE()) != NULL)
				{
					if (!AI_isDefended(-2)) // XXX check for other team's units?
					{
						iMinTurns = std::min(iMinTurns, 5);
					}
				}
			}

			if (eProductionUnitAI == UNITAI_SETTLE)
			{
				if (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLE) == 0)
				{
					if (!(GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble()))
					{
						if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
						{
							iMinTurns = std::min(iMinTurns, 5);
							bEssential = true;
							bGrowth = true;
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_SETTLER_SEA)
			{
				if (pWaterArea != NULL)
				{
					if (pWaterArea->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLER_SEA) == 0)
					{
						if (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLE) > 0)
						{
							iMinTurns = std::min(iMinTurns, 5);
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()) > (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_WORKER) * 2))
				{
					iMinTurns = std::min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER_SEA)
			{
				if (AI_neededSeaWorkers() > 0)
				{
					iMinTurns = std::min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
				}
			}

			// adjust for game speed
			if (NO_UNIT != getProductionUnit())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
			}
			else if (NO_BUILDING != getProductionBuilding())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
			}
			else if (NO_PROJECT != getProductionProject())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent();
			}
			else
			{
				iMinTurns *= 100;
			}

			iMinTurns /= 100;

			//this overrides everything.
			if (bGrowth)
			{
				int iHurryGold = hurryGold((HurryTypes)iI);
				if ((iHurryGold > 0) && ((iHurryGold * 16) < GET_PLAYER(getOwnerINLINE()).getGold()))
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
					if( gCityLogLevel >= 2 )
					{
						logBBAI("      City %S hurry gold at %d for growth when rich at %d", getName().GetCString(), iHurryGold, GET_PLAYER(getOwnerINLINE()).getGold() );
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					hurry((HurryTypes)iI);
					break;					
				}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/06/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
				if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
				{
					hurry((HurryTypes)iI);
					break;					
				}
			}
			if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
			{
*/
				// Only consider population hurry if that's actually what the city can do!!!
				if( (iHurryPopulation > 0) && (getPopulation() > iHurryPopulation) )
				{
					//BBAI TODO: could be (bEssential ? 100 : 80) instead
					if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
						if( gCityLogLevel >= 2 )
						{
							logBBAI("      City %S hurry pop at %d for growth with bad tiles with pop %d", getName().GetCString(), iHurryPopulation, getPopulation() );
						}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
						hurry((HurryTypes)iI);
						break;					
					}
				}				
			}

			if ((iHurryPopulation > 0) && (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation)))
			{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
				if (getProductionTurnsLeft() > iMinTurns)
				{
					bWait = isHuman();
					
					if ((iHurryPopulation * 3) > (getProductionTurnsLeft() * 2))
					{
						bWait = true;
					}

					if (!bWait)
					{
						if (iHurryAngerLength > 0)
						{
							//is the whip just too small or the population just too reduced to bother?
							if (!bEssential && ((iHurryPopulation < (1 + GC.getDefineINT("HURRY_POP_ANGER"))) || ((getPopulation() - iHurryPopulation) <= std::max(3, (getHighestPopulation() / 2)))))
							{
								bWait = true;
							}
							else
							{
								//sometimes it's worth whipping even with existing anger
								if (getHurryAngerTimer() > 1)
								{
									if (!bEssential)
									{
										bWait = true;
									}
									else if (GC.getDefineINT("HURRY_POP_ANGER") == iHurryPopulation && angryPopulation() > 0)
									{
										//ideally we'll whip something more expensive
										bWait = true;
									}
								}
							}

							//if the city is just lame then don't whip the poor thing
							//(it'll still get whipped when unhappy/unhealthy)
							if (!bWait && !bEssential)
							{
								int iFoodSurplus = 0;
								CvPlot * pLoopPlot;

								for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
								{
									if (iJ != CITY_HOME_PLOT)
									{
										pLoopPlot = getCityIndexPlot(iJ);

										if (pLoopPlot != NULL)
										{
											if (pLoopPlot->getWorkingCity() == this)
											{
												iFoodSurplus += std::max(0, pLoopPlot->getYield(YIELD_FOOD) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
											}
										}
									}
								}

								if (iFoodSurplus < 3)
								{
									bWait = true;
								}
							}
						}
					}

					if (!bWait)
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
						if( gCityLogLevel >= 2 )
						{
							logBBAI("      City %S hurry pop at %d with bad tiles and no reason to wait with pop %d", getName().GetCString(), iHurryPopulation, getPopulation() );
						}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
						hurry((HurryTypes)iI);
						break;
					}
				}
			}
		}
	}
}


// Improved use of emphasize by Blake, to go with his whipping strategy - thank you!
void CvCityAI::AI_doEmphasize()
{
	PROFILE_FUNC();

	FAssert(!isHuman());

	bool bFirstTech;
	bool bEmphasize;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	bool bCultureVictory = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	//Note from Blake:
	//Emphasis proved to be too clumsy to manage AI economies,
	//as such it's been nearly completely phased out by 
	//the AI_specialYieldMultiplier system which allows arbitary
	//value-boosts and works very well.
	//Ideally the AI should never use emphasis.
	int iI;
	
	if (GET_PLAYER(getOwnerINLINE()).getCurrentResearch() != NO_TECH)
	{
		bFirstTech = GET_PLAYER(getOwnerINLINE()).AI_isFirstTech(GET_PLAYER(getOwnerINLINE()).getCurrentResearch());
	}
	else
	{
		bFirstTech = false;
	}

	int iPopulationRank = findPopulationRank();

	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		bEmphasize = false;

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_FOOD) > 0)
		{

		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_PRODUCTION) > 0)
		{

		}
		
		if (AI_specialYieldMultiplier(YIELD_PRODUCTION) < 50)
		{
			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_COMMERCE) > 0)
			{
				if (bFirstTech)
				{
					bEmphasize = true;
				}
			}

			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getCommerceChange(COMMERCE_RESEARCH) > 0)
			{
				if (bFirstTech && !bCultureVictory)
				{
					if (iPopulationRank < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 4) + 1))
					{
						bEmphasize = true;
					}
				}
			}
			
			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).isGreatPeople())
			{
				int iHighFoodTotal = 0;
				int iHighFoodPlotCount = 0;
				int iHighHammerPlotCount = 0;
				int iHighHammerTotal = 0;
				int iGoodFoodSink = 0;
				int iFoodPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();
				for (int iPlot = 0; iPlot < NUM_CITY_PLOTS; iPlot++)
				{
					CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iPlot);
					if (pLoopPlot != NULL && pLoopPlot->getWorkingCity() == this)
					{
						int iFood = pLoopPlot->getYield(YIELD_FOOD);
						if (iFood > iFoodPerPop)
						{
							iHighFoodTotal += iFood;
							iHighFoodPlotCount++;
						}
						int iHammers = pLoopPlot->getYield(YIELD_PRODUCTION);
						if ((iHammers >= 3) && ((iHammers + iFood) >= 4))
						{
							iHighHammerPlotCount++;
							iHighHammerTotal += iHammers;
						}
						int iCommerce = pLoopPlot->getYield(YIELD_COMMERCE);
						if ((iCommerce * 2 + iHammers * 3) > 9)
						{
							iGoodFoodSink += std::max(0, iFoodPerPop - iFood);
						}
					}
				}
				
				if ((iHighFoodTotal + iHighFoodPlotCount - iGoodFoodSink) >= foodConsumption(true))
				{
					if ((iHighHammerPlotCount < 2) && (iHighHammerTotal < (getPopulation())))
					{
						if (AI_countGoodTiles(true, false, 100, true) < getPopulation())
						{
							bEmphasize = true;
						}
					}
				}
			}
		}

		AI_setEmphasize(((EmphasizeTypes)iI), bEmphasize);
	}
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/09/10                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
bool CvCityAI::AI_chooseUnit(UnitAITypes eUnitAI, int iOdds, bool bIgnoreNotUnitAIs)
{
	UnitTypes eBestUnit;

	if (eUnitAI != NO_UNITAI)
	{
		eBestUnit = AI_bestUnitAI(eUnitAI, false, NO_ADVISOR, bIgnoreNotUnitAIs);
	}
	else
	{
		eBestUnit = AI_bestUnit(false, NO_ADVISOR, &eUnitAI);
	}

	if (eBestUnit != NO_UNIT)
	{
		if( iOdds < 0 ||
			getUnitProduction(eBestUnit) > 0 ||
			GC.getGameINLINE().getSorenRandNum(100, "City AI choose unit") < iOdds )
		{
			pushOrder(ORDER_TRAIN, eBestUnit, eUnitAI, false, false, false);
			return true;
		}
	}

	return false;
}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

bool CvCityAI::AI_chooseUnit(UnitTypes eUnit, UnitAITypes eUnitAI)
{
	if (eUnit != NO_UNIT)
	{
		pushOrder(ORDER_TRAIN, eUnit, eUnitAI, false, false, false);
		return true;
	}
	return false;	
}


bool CvCityAI::AI_chooseDefender()
{
	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_SPECIAL, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(UNITAI_CITY_SPECIAL))
		{
			return true;
		}
	}

	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_COUNTER, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(UNITAI_CITY_COUNTER))
		{
			return true;
		}
	}

	if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
	{
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseLeastRepresentedUnit(UnitTypeWeightArray &allowedTypes, int iOdds)
{
	int iValue;

	UnitTypeWeightArray::iterator it;
	
 	std::multimap<int, UnitAITypes, std::greater<int> > bestTypes;
 	std::multimap<int, UnitAITypes, std::greater<int> >::iterator best_it;
 	
 	
	for (it = allowedTypes.begin(); it != allowedTypes.end(); it++)
	{
		iValue = it->second;
		iValue *= 750 + GC.getGameINLINE().getSorenRandNum(250, "AI choose least represented unit");
		iValue /= 1 + GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), it->first);
		bestTypes.insert(std::make_pair(iValue, it->first));
	}
 	
 	for (best_it = bestTypes.begin(); best_it != bestTypes.end(); best_it++)
 	{
		if (AI_chooseUnit(best_it->second, iOdds))
		{
			return true;
		}
 	}
	return false;
}

bool CvCityAI::AI_bestSpreadUnit(bool bMissionary, bool bExecutive, int iBaseChance, UnitTypes* eBestSpreadUnit, int* iBestSpreadUnitValue)
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	CvGame& kGame = GC.getGame();
	
	FAssert(eBestSpreadUnit != NULL && iBestSpreadUnitValue != NULL);

	int iBestValue = 0;

	if (bMissionary)
	{
		for (int iReligion = 0; iReligion < GC.getNumReligionInfos(); iReligion++)
		{
			ReligionTypes eReligion = (ReligionTypes)iReligion;
			if (isHasReligion(eReligion))
			{
				int iHasCount = kPlayer.getHasReligionCount(eReligion);
				FAssert(iHasCount > 0);
				int iRoll = (iHasCount > 4) ? iBaseChance : (((100 - iBaseChance) / iHasCount) + iBaseChance);
				if (kPlayer.AI_isDoStrategy(AI_STRATEGY_MISSIONARY))
				{
					iRoll *= (kPlayer.getStateReligion() == eReligion) ? 170 : 65;
					iRoll /= 100;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
				if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				{
					iRoll += 25;
				}
				else if (!kTeam.hasHolyCity(eReligion) && !(kPlayer.getStateReligion() == eReligion))
				{
					iRoll /= 2;
					if (kPlayer.isNoNonStateReligionSpread())
					{
						iRoll /= 2;
					}
				}

				if (iRoll > kGame.getSorenRandNum(100, "AI choose missionary"))
				{
					int iReligionValue = kPlayer.AI_missionaryValue(area(), eReligion);
					if (iReligionValue > 0)
					{
						for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

							if (eLoopUnit != NO_UNIT)
							{
								CvUnitInfo& kUnitInfo = GC.getUnitInfo(eLoopUnit);
								if (kUnitInfo.getReligionSpreads(eReligion) > 0)
								{
									if (canTrain(eLoopUnit))
									{
										int iValue = iReligionValue;
										iValue /= kUnitInfo.getProductionCost();
										
										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											*eBestSpreadUnit = eLoopUnit;
											*iBestSpreadUnitValue = iReligionValue;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (bExecutive)
	{
		for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); iCorporation++)
		{
			CorporationTypes eCorporation = (CorporationTypes)iCorporation;
			if (isActiveCorporation(eCorporation))
			{
				int iHasCount = kPlayer.getHasCorporationCount(eCorporation);
				FAssert(iHasCount > 0);
				int iRoll = (iHasCount > 4) ? iBaseChance : (((100 - iBaseChance) / iHasCount) + iBaseChance);
				if (!kTeam.hasHeadquarters(eCorporation))
				{
					iRoll /= 8;			
				}
				
				if (iRoll > kGame.getSorenRandNum(100, "AI choose executive"))
				{
					int iCorporationValue = kPlayer.AI_executiveValue(area(), eCorporation);
					if (iCorporationValue > 0)
					{
						for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

							if (eLoopUnit != NO_UNIT)
							{
								CvUnitInfo& kUnitInfo = GC.getUnitInfo(eLoopUnit);
								if (kUnitInfo.getCorporationSpreads(eCorporation) > 0)
								{
									if (canTrain(eLoopUnit))
									{
										int iValue = iCorporationValue;
										iValue /= kUnitInfo.getProductionCost();

										int iLoop;
										int iTotalCount = 0;
										int iPlotCount = 0;
										for (CvUnit* pLoopUnit = kPlayer.firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = kPlayer.nextUnit(&iLoop))
										{
											if ((pLoopUnit->AI_getUnitAIType() == UNITAI_MISSIONARY) && (pLoopUnit->getUnitInfo().getCorporationSpreads(eCorporation) > 0))
											{
												iTotalCount++;
												if (pLoopUnit->plot() == plot())
												{
													iPlotCount++;
												}
											}
										}
										iCorporationValue /= std::max(1, (iTotalCount / 4) + iPlotCount);

										int iCost = std::max(0, GC.getCorporationInfo(eCorporation).getSpreadCost() * (100 + GET_PLAYER(getOwnerINLINE()).calculateInflationRate()));
										iCost /= 100;
										
										if (kPlayer.getGold() >= iCost)
										{
											iCost *= GC.getDefineINT("CORPORATION_FOREIGN_SPREAD_COST_PERCENT");
											iCost /= 100;
											if (kPlayer.getGold() < iCost && iTotalCount > 1)
											{
												iCorporationValue /= 2;
											}
										}
										else if (iTotalCount > 1)
										{
											iCorporationValue /= 5;
										}
										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											*eBestSpreadUnit = eLoopUnit;
											*iBestSpreadUnitValue = iCorporationValue;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	return (*eBestSpreadUnit != NULL);
}

bool CvCityAI::AI_chooseBuilding(int iFocusFlags, int iMaxTurns, int iMinThreshold, int iOdds)
{
	BuildingTypes eBestBuilding;

	eBestBuilding = AI_bestBuildingThreshold(iFocusFlags, iMaxTurns, iMinThreshold);

	if (eBestBuilding != NO_BUILDING)
	{
		if( iOdds < 0 || 
			getBuildingProduction(eBestBuilding) > 0 ||
			GC.getGameINLINE().getSorenRandNum(100,"City AI choose building") < iOdds )
		{
			pushOrder(ORDER_CONSTRUCT, eBestBuilding, -1, false, false, false);
			return true;
		}
	}

	return false;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


bool CvCityAI::AI_chooseProject()
{
	ProjectTypes eBestProject;

	eBestProject = AI_bestProject();

	if (eBestProject != NO_PROJECT)
	{
		pushOrder(ORDER_CREATE, eBestProject, -1, false, false, false);
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseProcess(CommerceTypes eCommerceType)
{
	ProcessTypes eBestProcess;

	eBestProcess = AI_bestProcess(eCommerceType);

	if (eBestProcess != NO_PROCESS)
	{
		pushOrder(ORDER_MAINTAIN, eBestProcess, -1, false, false, false);
		return true;
	}

	return false;
}


// Returns true if a worker was added to a plot...
bool CvCityAI::AI_addBestCitizen(bool bWorkers, bool bSpecialists, int* piBestPlot, SpecialistTypes* peBestSpecialist)
{
	PROFILE_FUNC();

	bool bAvoidGrowth = AI_avoidGrowth();
	bool bIgnoreGrowth = AI_ignoreGrowth();
	bool bIsSpecialistForced = false;

	int iBestSpecialistValue = 0;
	SpecialistTypes eBestSpecialist = NO_SPECIALIST;
	SpecialistTypes eBestForcedSpecialist = NO_SPECIALIST;

	if (bSpecialists)
	{
		// count the total forced specialists
		int iTotalForcedSpecialists = 0;
		for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
			if (iForcedSpecialistCount > 0)
			{
				bIsSpecialistForced = true;
				iTotalForcedSpecialists += iForcedSpecialistCount;
			}
		}
		
		// if forcing any specialists, find the best one that we can still assign
		if (bIsSpecialistForced)
		{
			int iBestForcedValue = MIN_INT;
			
			int iTotalSpecialists = 1 + getSpecialistPopulation();
			for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
					if (iForcedSpecialistCount > 0)
					{
						int iSpecialistCount = getSpecialistCount((SpecialistTypes)iI);

						// the value is based on how close we are to our goal ratio forced/total
						int iForcedValue = ((iForcedSpecialistCount * 128) / iTotalForcedSpecialists) -  ((iSpecialistCount * 128) / iTotalSpecialists);
						if (iForcedValue >= iBestForcedValue)
						{
							int iSpecialistValue = AI_specialistValue((SpecialistTypes)iI, bAvoidGrowth, false);
							
							// if forced value larger, or if equal, does this specialist have a higher value
							if (iForcedValue > iBestForcedValue || iSpecialistValue > iBestSpecialistValue)
							{					
								iBestForcedValue = iForcedValue;
								iBestSpecialistValue = iSpecialistValue;
								eBestForcedSpecialist = ((SpecialistTypes)iI);
								eBestSpecialist = eBestForcedSpecialist;
							}
						}
					}
				}
			}
		}
		
		// if we do not have a best specialist yet, then just find the one with the best value
		if (eBestSpecialist == NO_SPECIALIST)
		{
			for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, false);
					if (iValue >= iBestSpecialistValue)
					{
						iBestSpecialistValue = iValue;
						eBestSpecialist = ((SpecialistTypes)iI);
					}
				}
			}
		}
	}

	int iBestPlotValue = 0;
	int iBestPlot = -1;
	if (bWorkers)
	{
		for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				if (!isWorkingPlot(iI))
				{
					CvPlot* pLoopPlot = getCityIndexPlot(iI);

					if (pLoopPlot != NULL)
					{
						if (canWork(pLoopPlot))
						{
							int iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ false, /*bIgnoreFood*/ false, bIgnoreGrowth);

							if (iValue > iBestPlotValue)
							{
								iBestPlotValue = iValue;
								iBestPlot = iI;
							}
						}
					}
				}
			}
		}
	}
	
	// if we found a plot to work
	if (iBestPlot != -1)
	{
		// if the best plot value is better than the best specialist, or if we forcing and we could not assign a forced specialst
		if (iBestPlotValue > iBestSpecialistValue || (bIsSpecialistForced && eBestForcedSpecialist == NO_SPECIALIST))
		{
			// do not work the specialist
			eBestSpecialist = NO_SPECIALIST;
		}
	}
	
	if (eBestSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eBestSpecialist, 1);
		if (piBestPlot != NULL)
		{
			FAssert(peBestSpecialist != NULL);
			*peBestSpecialist = eBestSpecialist;
			*piBestPlot = -1;
		}
		return true;
	}
	else if (iBestPlot != -1)
	{
		setWorkingPlot(iBestPlot, true);
		if (piBestPlot != NULL)
		{
			FAssert(peBestSpecialist != NULL);
			*peBestSpecialist = NO_SPECIALIST;
			*piBestPlot = iBestPlot;
			
		}
		return true;
	}

	return false;
}


// Returns true if a worker was removed from a plot...
bool CvCityAI::AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist)
{
	CvPlot* pLoopPlot;
	SpecialistTypes eWorstSpecialist;
	bool bAvoidGrowth;
	bool bIgnoreGrowth;
	int iWorstPlot;
	int iValue;
	int iWorstValue;
	int iI;
	
	// if we are using more specialists than the free ones we get
	if (extraFreeSpecialists() < 0)
	{
		// does generic 'citizen' specialist exist?
		if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
		{
			// is ignore something other than generic citizen?
			if (eIgnoreSpecialist != GC.getDefineINT("DEFAULT_SPECIALIST"))
			{
				// do we have at least one more generic citizen than we are forcing?
				if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > getForceSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))))
				{
					// remove the extra generic citzen
					changeSpecialistCount(((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))), -1);
					return true;
				}
			}
		}
	}

	bAvoidGrowth = AI_avoidGrowth();
	bIgnoreGrowth = AI_ignoreGrowth();

	iWorstValue = MAX_INT;
	eWorstSpecialist = NO_SPECIALIST;
	iWorstPlot = -1;

	// if we are using more specialists than the free ones we get
	if (extraFreeSpecialists() < 0)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (eIgnoreSpecialist != iI)
			{
				if (getSpecialistCount((SpecialistTypes)iI) > getForceSpecialistCount((SpecialistTypes)iI))
				{
					iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ true);

					if (iValue < iWorstValue)
					{
						iWorstValue = iValue;
						eWorstSpecialist = ((SpecialistTypes)iI);
						iWorstPlot = -1;
					}
				}
			}
		}
	}
	
	// check all the plots we working
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			if (isWorkingPlot(iI))
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ true, /*bIgnoreFood*/ false, bIgnoreGrowth);

					if (iValue < iWorstValue)
					{
						iWorstValue = iValue;
						eWorstSpecialist = NO_SPECIALIST;
						iWorstPlot = iI;
					}
				}
			}
		}
	}

	if (eWorstSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eWorstSpecialist, -1);
		return true;
	}
	else if (iWorstPlot != -1)
	{
		setWorkingPlot(iWorstPlot, false);
		return true;
	}
	
	// if we still have not removed one, then try again, but do not ignore the one we were told to ignore
	if (extraFreeSpecialists() < 0)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (getSpecialistCount((SpecialistTypes)iI) > 0)
			{
				iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ true);

				if (iValue < iWorstValue)
				{
					iWorstValue = iValue;
					eWorstSpecialist = ((SpecialistTypes)iI);
					iWorstPlot = -1;
				}
			}
		}
	}

	if (eWorstSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eWorstSpecialist, -1);
		return true;
	}

	return false;
}


void CvCityAI::AI_juggleCitizens()
{
	bool bAvoidGrowth = AI_avoidGrowth();
	bool bIgnoreGrowth = AI_ignoreGrowth();
	
	// one at a time, remove the worst citizen, then add the best citizen
	// until we add back the same one we removed
	for (int iPass = 0; iPass < 2; iPass++)
	{
		bool bCompletedChecks = false;
		int iCount = 0;

		std::vector<int> aWorstPlots;

		while (!bCompletedChecks)
		{
			int iLowestValue = MAX_INT;
			int iWorstPlot = -1;
			int iValue;

			for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				if (iI != CITY_HOME_PLOT)
				{
					if (isWorkingPlot(iI))
					{
						CvPlot* pLoopPlot = getCityIndexPlot(iI);

						if (pLoopPlot != NULL)
						{
								iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ true, /*bIgnoreFood*/ false, bIgnoreGrowth, (iPass == 0));

								// use <= so that we pick the last one that is lowest, to avoid infinite loop with AI_addBestCitizen
								if (iValue <= iLowestValue)
								{
									iLowestValue = iValue;
									iWorstPlot = iI;
								}
							}
						}
					}
				}

			// if no worst plot, or we looped back around and are trying to remove the first plot we removed, stop
			if (iWorstPlot == -1 || std::find(aWorstPlots.begin(), aWorstPlots.end(), iWorstPlot) != aWorstPlots.end())
			{
				bCompletedChecks = true;
			}
			else
			{
				// if this the first worst plot, remember it
				aWorstPlots.push_back(iWorstPlot);

				setWorkingPlot(iWorstPlot, false);

				if (AI_addBestCitizen(true, true))
				{
					if (isWorkingPlot(iWorstPlot))
					{
						bCompletedChecks = true;
					}
				}
			}

			iCount++;
			if (iCount > (NUM_CITY_PLOTS + 1))
			{
				FAssertMsg(false, "infinite loop");
				break; // XXX
			}
		}

		if ((iPass == 0) && (foodDifference(false) >= 0))
		{
			//good enough, the starvation code
			break;
		}
	}
}


bool CvCityAI::AI_potentialPlot(short* piYields)
{
	int iNetFood = piYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION();

	if (iNetFood < 0)
	{
 		if (piYields[YIELD_FOOD] == 0)
		{
			if (piYields[YIELD_PRODUCTION] + piYields[YIELD_COMMERCE] < 2)
			{
				return false;
			}
		}
		else
		{
			if (piYields[YIELD_PRODUCTION] + piYields[YIELD_COMMERCE] == 0)
			{
				return false;
			}
		}
	}

	return true;
}


bool CvCityAI::AI_foodAvailable(int iExtra)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	bool abPlotAvailable[NUM_CITY_PLOTS];
	int iFoodCount;
	int iPopulation;
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iFoodCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		abPlotAvailable[iI] = false;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (iI == CITY_HOME_PLOT)
			{
				iFoodCount += pLoopPlot->getYield(YIELD_FOOD);
			}
			else if ((pLoopPlot->getWorkingCity() == this) && AI_potentialPlot(pLoopPlot->getYield()))
			{
				abPlotAvailable[iI] = true;
			}
		}
	}

	iPopulation = (getPopulation() + iExtra);

	while (iPopulation > 0)
	{
		iBestValue = 0;
		iBestPlot = CITY_HOME_PLOT;

		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (abPlotAvailable[iI])
			{
				iValue = getCityIndexPlot(iI)->getYield(YIELD_FOOD);

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					iBestPlot = iI;
				}
			}
		}

		if (iBestPlot != CITY_HOME_PLOT)
		{
			iFoodCount += iBestValue;
			abPlotAvailable[iBestPlot] = false;
		}
		else
		{
			break;
		}

		iPopulation--;
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		iFoodCount += (GC.getSpecialistInfo((SpecialistTypes)iI).getYieldChange(YIELD_FOOD) * getFreeSpecialistCount((SpecialistTypes)iI));
	}

	if (iFoodCount < foodConsumption(false, iExtra))
	{
		return false;
	}

	return true;
}


int CvCityAI::AI_yieldValue(short* piYields, short* piCommerceYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation, bool bWorkerOptimization)
{
	const int iBaseProductionValue = 15;
	const int iBaseCommerceValue = 7;
	
	const int iMaxFoodValue = (3 * iBaseProductionValue) - 1;

	int aiYields[NUM_YIELD_TYPES];
	int aiCommerceYieldsTimes100[NUM_COMMERCE_TYPES];

	int iExtraProductionModifier = 0;
	int iBaseProductionModifier = 100;
	
	bool bEmphasizeFood = AI_isEmphasizeYield(YIELD_FOOD);
	bool bFoodIsProduction = isFoodProduction();
	bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();
	
	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		aiCommerceYieldsTimes100[iJ] = 0;
	}

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		if (piYields[iI] == 0)
		{
			aiYields[iI] = 0;
		}
		else
		{
			// Get yield for city after adding/removing the citizen in question
			int iOldCityYield = getBaseYieldRate((YieldTypes)iI);
			int iNewCityYield = (bRemove ? (iOldCityYield - piYields[iI]) : (iOldCityYield + piYields[iI]));
			int iModifier = getBaseYieldRateModifier((YieldTypes)iI);
			if (iI == YIELD_PRODUCTION)
			{
				iBaseProductionModifier = iModifier;
				iExtraProductionModifier = getProductionModifier();
				iModifier += iExtraProductionModifier;
			}

			iNewCityYield = (iNewCityYield * iModifier) / 100;
			iOldCityYield = (iOldCityYield * iModifier) / 100;

			// The yield of the citizen in question is the difference of total yields
			// to account for rounding of modifiers
			aiYields[iI] = (bRemove ? (iOldCityYield - iNewCityYield) : (iNewCityYield - iOldCityYield));
		}
	}
			
	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		int iModifier = getTotalCommerceRateModifier((CommerceTypes)iJ);
				    
		int iCommerceTimes100 = aiYields[YIELD_COMMERCE] * GET_PLAYER(getOwnerINLINE()).getCommercePercent((CommerceTypes)iJ);
		if (piCommerceYields != NULL)
		{
			iCommerceTimes100 += piCommerceYields[iJ] * 100;
		}
		aiCommerceYieldsTimes100[iJ] += (iCommerceTimes100 * iModifier) / 100;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       07/09/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
/* original BTS code
	if (isProductionProcess() && !bWorkerOptimization)
	{
		for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
		{
			aiCommerceYieldsTimes100[iJ] += GC.getProcessInfo(getProductionProcess()).getProductionToCommerceModifier(iJ) * aiYields[YIELD_PRODUCTION];
		}

		aiYields[YIELD_PRODUCTION] = 0;
	}
*/
	// Above code causes governor and AI to heavily weight food when building any form of commerce,
	// which is not expected by human and does not seem to produce better results for AI either.  
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	
	// should not really use this much, but making it accurate
	aiYields[YIELD_COMMERCE] = 0;
	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		aiYields[YIELD_COMMERCE] += aiCommerceYieldsTimes100[iJ] / 100;
	}

	int iValue = 0;
	int iSlaveryValue = 0;
	
	int iFoodGrowthValue = 0;
	int iFoodGPPValue = 0;
	
	if (!bIgnoreFood && aiYields[YIELD_FOOD] != 0)
	{
		// tiny food factor, to ensure that even when we don't want to grow, 
		// we still prefer more food if everything else is equal
		iValue += (aiYields[YIELD_FOOD] * 1);

		int iFoodPerTurn = (foodDifference(false) - ((bRemove) ? aiYields[YIELD_FOOD] : 0));
		int iFoodLevel = getFood();
		int iFoodToGrow = growthThreshold();
		int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, 0);
		int iHappinessLevel = (isNoUnhappiness() ? std::max(3, iHealthLevel + 5) : happyLevel() - unhappyLevel(0));
		int iPopulation = getPopulation();
		int	iExtraPopulationThatCanWork = std::min(iPopulation - range(-iHappinessLevel, 0, iPopulation) + std::min(0, extraFreeSpecialists()) , NUM_CITY_PLOTS) - getWorkingPopulation() + ((bRemove) ? 1 : 0);
		int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();

		int iAdjustedFoodDifference = (getYieldRate(YIELD_FOOD) + std::min(0, iHealthLevel)) - ((iPopulation + std::min(0, iHappinessLevel) - ((bRemove) ? 1 : 0)) * iConsumtionPerPop);
		
		// if we not human, allow us to starve to half full if avoiding growth
		if (!bIgnoreStarvation)
		{
			int iStarvingAllowance = 0;
			if (bAvoidGrowth && !isHuman())
			{
				iStarvingAllowance = std::max(0, (iFoodLevel - std::max(1, ((9 * iFoodToGrow) / 10))));
			}
			
			if ((iStarvingAllowance < 1) && (iFoodLevel > ((iFoodToGrow * 75) / 100)))
			{
				iStarvingAllowance = 1;
			}
			
			// if still starving
			if ((iFoodPerTurn + iStarvingAllowance) < 0)
			{
				// if working plots all like this one will save us from starving
				if (std::max(0, iExtraPopulationThatCanWork * aiYields[YIELD_FOOD]) >= -iFoodPerTurn)
				{
					// if this is high food, then we want to pick it first, this will allow us to pick some great non-food later
					int iHighFoodThreshold = std::min(getBestYieldAvailable(YIELD_FOOD), iConsumtionPerPop + 1);				
					if (iFoodPerTurn <= (AI_isEmphasizeGreatPeople() ? 0 : -iHighFoodThreshold) && aiYields[YIELD_FOOD] >= iHighFoodThreshold)
					{
						// value all the food that will contribute to not starving
						iValue += 2048 * std::min(aiYields[YIELD_FOOD], -iFoodPerTurn);
					}
					else
					{
						// give a huge boost to this plot, but not based on how much food it has
						// ie, if working a bunch of 1f 7h plots will stop us from starving, then do not force working unimproved 2f plot
						iValue += 2048;
					}
				}
				else
				{
					// value food high(32), but not forced
					iValue += 32 * std::min(aiYields[YIELD_FOOD], -iFoodPerTurn);
				}
			}
		}
		
		// if food isn't production, then adjust for growth
		if (bWorkerOptimization || !bFoodIsProduction)
		{
			int iPopToGrow = 0;
			if (!bAvoidGrowth)
			{
				// only do relative checks on food if we want to grow AND we not emph food
				// the emp food case will just give a big boost to all food under all circumstances
				if (bWorkerOptimization || (!bIgnoreGrowth))// && !bEmphasizeFood))
				{
					// also avail: iFoodLevel, iFoodToGrow
					
					// adjust iFoodPerTurn assuming that we work plots all equal to iConsumtionPerPop
					// that way it is our guesstimate of how much excess food we will have
					iFoodPerTurn += (iExtraPopulationThatCanWork * iConsumtionPerPop);
					
					// we have less than 10 extra happy, do some checks to see if we can increase it
					if (iHappinessLevel < 10)
					{
						// if we have anger becase no military, do not count it, on the assumption that it will 
						// be remedied soon, and that we still want to grow
						if (getMilitaryHappinessUnits() == 0)
						{
							if (GET_PLAYER(getOwnerINLINE()).getNumCities() > 2)
							{
								iHappinessLevel += ((GC.getDefineINT("NO_MILITARY_PERCENT_ANGER") * (iPopulation + 1)) / GC.getPERCENT_ANGER_DIVISOR());
							}
						}

						// currently we can at most increase happy by 2 in the following checks
						const int kMaxHappyIncrease = 2;

						// if happy is large enough so that it will be over zero after we do the checks
						int iNewFoodPerTurn = iFoodPerTurn + aiYields[YIELD_FOOD] - iConsumtionPerPop;
						if ((iHappinessLevel + kMaxHappyIncrease) > 0 && iNewFoodPerTurn > 0)
						{
							int iApproxTurnsToGrow = (iNewFoodPerTurn > 0) ? ((iFoodToGrow - iFoodLevel) / iNewFoodPerTurn) : MAX_INT;

							// do we have hurry anger?
							int iHurryAngerTimer = getHurryAngerTimer();
							if (iHurryAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iHurryAngerTimer % flatHurryAngerLength();
								
								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}

							// do we have conscript anger?
							int iConscriptAngerTimer = getConscriptAngerTimer();
							if (iConscriptAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iConscriptAngerTimer % flatConscriptAngerLength();
								
								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}

							// do we have defy resolution anger?
							int iDefyResolutionAngerTimer = getDefyResolutionAngerTimer();
							if (iDefyResolutionAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iDefyResolutionAngerTimer % flatDefyResolutionAngerLength();

								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}
						}
					}
					
					if (bEmphasizeFood)
					{
						//If we are emphasize food, pay less heed to caps.
						iHealthLevel += 5;
						iHappinessLevel += 2;
					}

					bool bBarFull = (iFoodLevel + iFoodPerTurn /*+ aiYields[YIELD_FOOD]*/ > ((90 * iFoodToGrow) / 100));

					int iPopToGrow = std::max(0, iHappinessLevel);
					int iGoodTiles = AI_countGoodTiles(iHealthLevel > 0, true, 50, true);
					iGoodTiles += AI_countGoodSpecialists(iHealthLevel > 0);
					iGoodTiles += bBarFull ? 0 : 1;

					if (!bEmphasizeFood)
					{
						iPopToGrow = std::min(iPopToGrow, iGoodTiles + ((bRemove) ? 1 : 0));
					}

					// if we have growth pontential, fill food bar to 85%
					bool bFillingBar = false;
					if (iPopToGrow == 0 && iHappinessLevel >= 0 && iGoodTiles >= 0 && iHealthLevel >= 0)
					{
						if (!bBarFull)
						{
							if (AI_specialYieldMultiplier(YIELD_PRODUCTION) < 50)
							{
								bFillingBar = true;
							}
						}
					}
					
					if (getPopulation() < 3)
					{
						iPopToGrow = std::max(iPopToGrow, 3 - getPopulation());
						iPopToGrow += 2;
					}

					// if we want to grow
					if (iPopToGrow > 0 || bFillingBar)
					{
						
						// will multiply this by factors
						iFoodGrowthValue = aiYields[YIELD_FOOD];
						if (iHealthLevel < (bFillingBar ? 0 : 1))
						{
							iFoodGrowthValue--;
						}

						// (range 1-25) - we want to grow more if we have a lot of growth to do
						// factor goes up like this: 0:1, 1:8, 2:9, 3:10, 4:11, 5:13, 6:14, 7:15, 8:16, 9:17, ... 17:25
						int iFactorPopToGrow;

						if (iPopToGrow < 1 || bFillingBar)
							iFactorPopToGrow = 20 - (10 * (iFoodLevel + iFoodPerTurn + aiYields[YIELD_FOOD])) / iFoodToGrow;
						else if (iPopToGrow < 7)
							iFactorPopToGrow = 17 + 3 * iPopToGrow;
						else
							iFactorPopToGrow = 41;
						
						iFoodGrowthValue *= iFactorPopToGrow;
						
						//If we already grow somewhat fast, devalue further food
						//Remember growth acceleration is not dependent on food eaten per 
						//pop, 4f twice as fast as 2f twice as fast as 1f...
						int iHighGrowthThreshold = 2 + std::max(std::max(0, 5 - getPopulation()), (iPopToGrow + 1) / 2);
						if (bEmphasizeFood)
						{
							iHighGrowthThreshold *= 2;
						}
						
						if (iFoodPerTurn > iHighGrowthThreshold)
						{
							iFoodGrowthValue *= 25 + ((75 * iHighGrowthThreshold) / iFoodPerTurn);
							iFoodGrowthValue /= 100;
						}
					}
				}
			
				//very high food override
				if ((isHuman()) && ((iPopToGrow > 0) || bCanPopRush))
				{
					//very high food override
					int iTempValue = std::max(0, 30 * aiYields[YIELD_FOOD] - 15 * iConsumtionPerPop);
					iTempValue *= std::max(0, 3 * iConsumtionPerPop - iAdjustedFoodDifference);
					iTempValue /= 3 * iConsumtionPerPop;
					if (iHappinessLevel < 0)
					{
						iTempValue *= 2;
						iTempValue /= 1 + 2 * -iHappinessLevel;
					}
					iFoodGrowthValue += iTempValue;
				}
				//Slavery Override
				if (bCanPopRush && (iHappinessLevel > 0))
				{
					iSlaveryValue = 30 * 14 * std::max(0, aiYields[YIELD_FOOD] - ((iHealthLevel < 0) ? 1 : 0));
					iSlaveryValue /= std::max(10, (growthThreshold() * (100 - getMaxFoodKeptPercent())));
					
					iSlaveryValue *= 100;
					iSlaveryValue /= getHurryCostModifier(true);
					
					iSlaveryValue *= iConsumtionPerPop * 2;
					iSlaveryValue /= iConsumtionPerPop * 2 + std::max(0, iAdjustedFoodDifference);
				}
				
				//Great People Override
				if ((iExtraPopulationThatCanWork > 1) && AI_isEmphasizeGreatPeople())
				{
					int iAdjust = iConsumtionPerPop;
					if (iFoodPerTurn == 0)
					{
						iAdjust -= 1;
					}
					iFoodGPPValue += std::max(0, aiYields[YIELD_FOOD] - iAdjust) * std::max(0, (12 + 5 * std::min(0, iHappinessLevel)));
				}
			}
		}
	}
	
	
	int iProductionValue = 0;
	int iCommerceValue = 0;
	int iFoodValue = std::min(iFoodGrowthValue, iMaxFoodValue * aiYields[YIELD_FOOD]);
	// if food is production, the count it
	int adjustedYIELD_PRODUCTION = (((bFoodIsProduction) ? aiYields[YIELD_FOOD] : 0) + aiYields[YIELD_PRODUCTION]);
			
	// value production medium(15)
	iProductionValue += (adjustedYIELD_PRODUCTION * iBaseProductionValue);
	if (!isProduction() && !isHuman())
	{
		iProductionValue /= 2;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/18/09                                jdog5000      */
/*                                                                                              */
/* City AI				                                                                         */
/************************************************************************************************/
	// If city has more than enough food, but very little production, add large value to production
	// Particularly helps coastal cities with plains forests
	if( aiYields[YIELD_PRODUCTION] > 0 )
	{
		if( !bFoodIsProduction && isProduction() )
		{
			if( foodDifference(false) >= GC.getFOOD_CONSUMPTION_PER_POPULATION() )
			{
				if( getYieldRate(YIELD_PRODUCTION) < (1 + getPopulation()/3) )
				{
					iValue += 128 + 8 * aiYields[YIELD_PRODUCTION];
				}
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	// value commerce low(6)

	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (aiCommerceYieldsTimes100[iI] != 0)
		{
			int iCommerceWeight = GET_PLAYER(getOwnerINLINE()).AI_commerceWeight((CommerceTypes)iI);
			if (AI_isEmphasizeCommerce((CommerceTypes)iI))
			{
				iCommerceWeight *= 200;
				iCommerceWeight /= 100;
			}
			if (iI == COMMERCE_CULTURE)
			{
				if (getCultureLevel() <= (CultureLevelTypes) 1)
				{
					iCommerceValue += (15 * aiCommerceYieldsTimes100[iI]) / 100;
				}
			}
			iCommerceValue += (iCommerceWeight * (aiCommerceYieldsTimes100[iI] * iBaseCommerceValue) * GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI)) / 1000000;
		}
	}
/*
	if (!bWorkerOptimization && bEmphasizeFood)
	{
		if (!bFoodIsProduction)
		{
			// value food extremely high(180)
			iFoodValue *= 125;
			iFoodValue /= 100;
		}
	}
	
	if (!bWorkerOptimization && AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		// value production high(80)
		iProductionValue += (adjustedYIELD_PRODUCTION * 80);
	}
*/
	//Slavery translation
	if ((iSlaveryValue > 0) && (iSlaveryValue > iFoodValue))
	{
		//treat the food component as production
		iFoodValue = 0;
	}
	else
	{
		//treat it as just food
		iSlaveryValue = 0;
	}
	
	iFoodValue += iFoodGPPValue;
/*
	if (!bWorkerOptimization && AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			iCommerceValue += ((iCommerceYields[iI] * 40) * GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI)) / 100;
		}
	}

	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		if (!bWorkerOptimization && AI_isEmphasizeCommerce((CommerceTypes) iJ))
		{
			// value the part of our commerce that goes to our emphasis medium (40)
			iCommerceValue += (iCommerceYields[iJ] * 40);
		}
	}
*/
	//Lets have some fun with the multipliers, this basically bluntens the impact of
	//massive bonuses.....
	
	//normalize the production... this allows the system to account for rounding
	//and such while preventing an "out to lunch smoking weed" scenario with
	//unusually high transient production modifiers.
	//Other yields don't have transient bonuses in quite the same way.

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/16/10                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
	// Rounding can be a problem, particularly for small commerce amounts.  Added safe guards to make
	// sure commerce is counted, even if just a tiny amount.
	if (AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		iProductionValue *= 130;
		iProductionValue /= 100;
		
		if (isFoodProduction())
		{
			iFoodValue *= 130;
			iFoodValue /= 100;
		}
		
		if (!AI_isEmphasizeYield(YIELD_COMMERCE) && iCommerceValue > 0)
		{
			iCommerceValue *= 60;
			iCommerceValue /= 100;
			iCommerceValue = std::max(1, iCommerceValue);
		}
		if (!AI_isEmphasizeYield(YIELD_FOOD) && iFoodValue > 0)
		{
			iFoodValue *= 75;
			iFoodValue /= 100;
			iFoodValue = std::max(1, iFoodValue);
		}
	}
	if (AI_isEmphasizeYield(YIELD_FOOD))
	{
		if (!isFoodProduction())
		{
			iFoodValue *= 130;
			iFoodValue /= 100;
			iSlaveryValue *= 130;
			iSlaveryValue /= 100;
		}
	}
	if (AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		iCommerceValue *= 130;
		iCommerceValue /= 100;
		if (!AI_isEmphasizeYield(YIELD_PRODUCTION) && iProductionValue > 0)
		{
			iProductionValue *= 75;
			iProductionValue /= 100;
			iProductionValue = std::max(1,iProductionValue);
		}
		if (!AI_isEmphasizeYield(YIELD_FOOD) && iFoodValue > 0)
		{
			//Don't supress twice.
			if (!AI_isEmphasizeYield(YIELD_PRODUCTION))
			{
				iFoodValue *= 80;
				iFoodValue /= 100;
				iFoodValue = std::max(1, iFoodValue);
			}
		}
	}
		
	if( iProductionValue > 0 )
	{
		if (isFoodProduction())
		{
			iProductionValue *= 100 + (bWorkerOptimization ? 0 : AI_specialYieldMultiplier(YIELD_PRODUCTION));
			iProductionValue /= 100;		
		}
		else
		{
			iProductionValue *= iBaseProductionModifier;
			iProductionValue /= (iBaseProductionModifier + iExtraProductionModifier);
			
			iProductionValue += iSlaveryValue;
			iProductionValue *= (100 + (bWorkerOptimization ? 0 : AI_specialYieldMultiplier(YIELD_PRODUCTION)));
			
			iProductionValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_PRODUCTION);
		}
	
		iValue += std::max(1,iProductionValue);
	}
	
	if( iCommerceValue > 0 )
	{
		iCommerceValue *= (100 + (bWorkerOptimization ? 0 : AI_specialYieldMultiplier(YIELD_COMMERCE)));
		iCommerceValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_COMMERCE);
		iValue += std::max(1, iCommerceValue);
	}
//	
	if( iFoodValue > 0 )
	{
		iFoodValue *= 100;
		iFoodValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_FOOD);
		iValue += std::max(1, iFoodValue);
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	
	return iValue;
}


int CvCityAI::AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation)
{
	PROFILE_FUNC();

	short aiYields[NUM_YIELD_TYPES];
	ImprovementTypes eCurrentImprovement;
	ImprovementTypes eFinalImprovement;
	int iYieldDiff;
	int iValue;
	int iI;
	int iTotalDiff;

	iValue = 0;
	iTotalDiff = 0;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = pPlot->getYield((YieldTypes)iI);
	}

	eCurrentImprovement = pPlot->getImprovementType();
	eFinalImprovement = NO_IMPROVEMENT;

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		eFinalImprovement = finalImprovementUpgrade(eCurrentImprovement);
	}
	
	
	int iYieldValue = (AI_yieldValue(aiYields, NULL, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation) * 100);

	if (eFinalImprovement != NO_IMPROVEMENT)
	{
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			iYieldDiff = (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iI), getOwnerINLINE()) - pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes)iI), getOwnerINLINE()));
			aiYields[iI] += iYieldDiff;
		}
		int iFinalYieldValue = (AI_yieldValue(aiYields, NULL, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation) * 100);
		
		if (iFinalYieldValue > iYieldValue)
		{
			iYieldValue = (40 * iYieldValue + 60 * iFinalYieldValue) / 100;
		}
		else
		{
			iYieldValue = (60 * iYieldValue + 40 * iFinalYieldValue) / 100;
		}
	}
	// unless we are emph food (and also food not production)
	if (!(AI_isEmphasizeYield(YIELD_FOOD) && !isFoodProduction()))
		// if this plot is super bad (less than 2 food and less than combined 2 prod/commerce
		if (!AI_potentialPlot(aiYields))
			// undervalue it even more!
			iYieldValue /= 16;
	iValue += iYieldValue;

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		if (pPlot->getBonusType(getTeam()) == NO_BONUS) // XXX double-check CvGame::doFeature that the checks are the same...
		{
			for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes) iI).getTechReveal())))
				{
					if (GC.getImprovementInfo(eCurrentImprovement).getImprovementBonusDiscoverRand(iI) > 0)
					{
						iValue += 35;
					}
				}
			}
		}
	}

	if ((eCurrentImprovement != NO_IMPROVEMENT) && (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT))
	{
		iValue += 200;
		iValue -= pPlot->getUpgradeTimeLeft(eCurrentImprovement, NO_PLAYER);		
	}

	return iValue;
}


int CvCityAI::AI_experienceWeight()
{
	return ((getProductionExperience() + getDomainFreeExperience(DOMAIN_SEA)) * 2);
}


int CvCityAI::AI_buildUnitProb()
{
	int iProb;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/29/10                                jdog5000      */
/*                                                                                              */
/* City AI, Barbarian AI                                                                        */
/************************************************************************************************/
	iProb = (GC.getLeaderHeadInfo(getPersonalityType()).getBuildUnitProb() + AI_experienceWeight());

	if (!isBarbarian() && GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iProb /= 2;
	}
	else if( GET_TEAM(getTeam()).getHasMetCivCount(false) == 0 )
	{
		iProb /= 2;
	}
	// more units from cities with military production bonuses
	else
	{
		iProb += std::min(15,getMilitaryProductionModifier()/4);
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	return iProb;
}



// Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild, int iFoodPriority, int iProductionPriority, int iCommercePriority, bool bChop, int iHappyAdjust, int iHealthAdjust, int iFoodChange)
{
	PROFILE_FUNC();

	CvCity* pCity;
	int aiFinalYields[NUM_YIELD_TYPES];
	int aiDiffYields[NUM_YIELD_TYPES];
	int aiBestDiffYields[NUM_YIELD_TYPES];
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eBonus;
	BonusTypes eNonObsoleteBonus;

	bool bEmphasizeIrrigation = false;
	bool bIgnoreFeature;
	bool bHasBonusImprovement;
	bool bValid;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iI, iJ;

	if (piBestValue != NULL)
	{
		*piBestValue = 0;
	}
	if (peBestBuild != NULL)
	{
		*peBestBuild = NO_BUILD;
	}

	if (pPlot->getWorkingCity() != this)
	{
		return;
	}
	
	//When improving new plots only, count emphasis twice
	//helps to avoid too much tearing up of old improvements.
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/06/09                                jdog5000      */
/*                                                                                              */
/* Worker AI                                                                                    */
/************************************************************************************************/
	// AI no longer uses emphasis really, except for short term boosts to commerce.
	// Inappropriate to base improvements on short term goals.
	if( isHuman() )
	{
		if (pPlot->getImprovementType() == NO_IMPROVEMENT)
		{
			if (AI_isEmphasizeYield(YIELD_FOOD))
			{
				iFoodPriority *= 130;
				iFoodPriority /= 100;
			}
			if (AI_isEmphasizeYield(YIELD_PRODUCTION))
			{
				iProductionPriority *= 180;
				iProductionPriority /= 100;
			}
			if (AI_isEmphasizeYield(YIELD_COMMERCE))
			{
				iCommercePriority *= 180;
				iCommercePriority /= 100;
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	FAssertMsg(pPlot->getOwnerINLINE() == getOwnerINLINE(), "pPlot must be owned by this city's owner");

	eBonus = pPlot->getBonusType(getTeam());
	eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(getTeam());

	bHasBonusImprovement = false;

	if (eNonObsoleteBonus != NO_BONUS)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
/*************************************************************************************************/
/**	Forts Connect Resources									02/12/10				Xienwolf	**/
/*************************************************************************************************/
/* original code
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))
*/
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus) || GC.getImprovementInfo(pPlot->getImprovementType()).isActsAsCity())
/*************************************************************************************************/
/**	Forts Connect Resources					END													**/
/*************************************************************************************************/
			{
				bHasBonusImprovement = true;
			}
/********************************************************************************/
/* 	Bonus Improvement: Wait for Upgrade							Fuyu		    */
/********************************************************************************/
			//Fuyu: patience. We can wait 10 turns for upgrade
			else if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
			{
				if (GC.getImprovementInfo((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade())).isImprovementBonusTrade(eNonObsoleteBonus)
					|| GC.getImprovementInfo((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade())).isActsAsCity())
				{
					if (pPlot->getUpgradeTimeLeft(pPlot->getImprovementType(), getOwner()) <= 1 + ((9 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getImprovementPercent() * GC.getEraInfo(GC.getGameINLINE().getStartEra()).getImprovementPercent())/10000))
					{
						bHasBonusImprovement = true;
					}
				}
			}
/********************************************************************************/
/* 	Bonus Improvement: Wait for Upgrade							END			    */
/********************************************************************************/
		}
	}
	
	for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
	{
		aiBestDiffYields[iJ] = 0;
	}

	BuildTypes eForcedBuild = NO_BUILD;
	
	{	//If a worker is already building a build, force that Build.
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;

		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getBuildType() != NO_BUILD)
			{
				if (GC.getBuildInfo(pLoopUnit->getBuildType()).getImprovement() != NO_IMPROVEMENT)
				{
					eForcedBuild = pLoopUnit->getBuildType();
					break;
				}
			}
		}
	}

	iBestValue = 0;
	eBestBuild = NO_BUILD;
	
	int iClearFeatureValue = 0;
	
	if (pPlot->getFeatureType() != NO_FEATURE)
	{
		iClearFeatureValue = AI_clearFeatureValue(getCityPlotIndex(pPlot));
	}

	if (!bHasBonusImprovement)
	{
		bEmphasizeIrrigation = false;

        CvPlot* pAdjacentPlot;
        CvPlot* pAdjacentPlot2;
        BonusTypes eTempBonus;

        //It looks unwieldly but the code has to be rigid to avoid "worker ADD"
        //where they keep connecting then disconnecting a crops resource or building
        //multiple farms to connect a single crop resource.
        //isFreshWater is used to ensure invalid plots are pruned early, the inner loop
        //really doesn't run that often.

        //using logic along the lines of "Will irrigating me make crops wet"
        //wont really work... it does have to "am i the tile the crops want to be irrigated"

        //I optimized through the use of "isIrrigated" which is just checking a bool...
        //once everything is nicely irrigated, this code should be really fast...
        if ((pPlot->isIrrigated()) || (pPlot->isFreshWater() && pPlot->canHavePotentialIrrigation()))
        {
            for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
            {
                pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

                if ((pAdjacentPlot != NULL) && (pAdjacentPlot->getOwner() == getOwner()) && (pAdjacentPlot->isCityRadius()))
                {
                    if (!pAdjacentPlot->isFreshWater())
                    {
                        //check for a city? cities can conduct irrigation and that effect is quite
                        //useful... so I think irrigate cities.
                        if (pAdjacentPlot->isPotentialIrrigation())
                        {
                            CvPlot* eBestIrrigationPlot = NULL;


                            for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
                            {
                                pAdjacentPlot2 = plotDirection(pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), ((DirectionTypes)iJ));
                                if ((pAdjacentPlot2 != NULL) && (pAdjacentPlot2->getOwner() == getOwner()))
                                {
	                                eTempBonus = pAdjacentPlot2->getNonObsoleteBonusType(getTeam());
                                    if (pAdjacentPlot->isIrrigated())
                                    {
                                        //the irrigation has to be coming from somewhere
                                        if (pAdjacentPlot2->isIrrigated())
                                        {
                                            //if we find a tile which is already carrying irrigation
                                            //then lets not replace that one...
                                            eBestIrrigationPlot = pAdjacentPlot2;

                                            if ((pAdjacentPlot2->isCity()) || (eTempBonus != NO_BONUS) || (!pAdjacentPlot2->isCityRadius()))
                                            {
                                            	if (pAdjacentPlot2->isFreshWater())
                                            	{
                                                	//these are all ideal for irrigation chains so stop looking.
                                                	break;
                                            	}
                                            }
                                        }

                                    }
                                    else
                                    {
                                        if (pAdjacentPlot2->getNonObsoleteBonusType(getTeam()) == NO_BONUS)
                                        {
                                            if (pAdjacentPlot2->canHavePotentialIrrigation() && pAdjacentPlot2->isIrrigationAvailable())
                                            {
                                                //could use more sophisticated logic
                                                //however this would rely on things like smart irrigation chaining
                                                //of out-of-city plots
                                                eBestIrrigationPlot = pAdjacentPlot2;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }

                            if (pPlot == eBestIrrigationPlot)
                            {
                                bEmphasizeIrrigation = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
	}
	
		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			eImprovement = ((ImprovementTypes)iI);

			iBestTempBuildValue = 0;
			eBestTempBuild = NO_BUILD;

			bIgnoreFeature = false;
			bValid = false;

			if (eImprovement == pPlot->getImprovementType())
			{
				bValid = true;
			}
			else
			{
				if (eForcedBuild != NO_BUILD)
				{
					if (GC.getBuildInfo(eForcedBuild).getImprovement() == eImprovement)
					{
						eBestTempBuild = eForcedBuild;
					}
				}
				else
				{
					for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
					{
						eBuild = ((BuildTypes)iJ);

						if (GC.getBuildInfo(eBuild).getImprovement() == eImprovement)
						{
							if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
							{
								iValue = 10000;

								iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

								// XXX feature production???

								if (iValue > iBestTempBuildValue)
								{
									iBestTempBuildValue = iValue;
									eBestTempBuild = eBuild;
								}
							}
						}
					}
				}

				if (eBestTempBuild != NO_BUILD)
				{
					bValid = true;

					if (pPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							bIgnoreFeature = true;

							if (GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
							{
								if (eNonObsoleteBonus == NO_BONUS)
								{
									if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
									{
										bValid = false;
									}
									else if (healthRate() < 0 && GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() > 0)
									{
										bValid = false;
									}
									else if (GET_PLAYER(getOwnerINLINE()).getFeatureHappiness(pPlot->getFeatureType()) > 0)
									{
										bValid = false;
									}
								}
							}
						}
					}
				}
			}

			if (bValid)
			{
				eFinalImprovement = finalImprovementUpgrade(eImprovement);

				if (eFinalImprovement == NO_IMPROVEMENT)
				{
					eFinalImprovement = eImprovement;
				}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD																			*/
/*																								*/
/* City AI, FinalImprovement part moved here from inside yield loop (Fuyu)						*/
/************************************************************************************************/
				ImprovementTypes eCurImprovement = pPlot->getImprovementType();
				ImprovementTypes eCurFinalImprovement = NO_IMPROVEMENT;
				if( eCurImprovement != NO_IMPROVEMENT )
				{
					eCurFinalImprovement = finalImprovementUpgrade(eCurImprovement);
					if (eCurFinalImprovement == NO_IMPROVEMENT)
					{
						eCurFinalImprovement = eCurImprovement;
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				iValue = 0;
				
				if (eBonus != NO_BONUS)
				{
/*************************************************************************************************/
/**	Forts Connect Resources but should never be built					30.07.2010		Fuyu	**/
/*************************************************************************************************/
					if (eNonObsoleteBonus != NO_BONUS)
					{
						if (!bHasBonusImprovement)
						{
							if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
							{
								if (!(GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
								{
									//reduced value for temporary solutions
									if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo(eNonObsoleteBonus).getTechCityTrade()))))
									{
										iValue++;
									}
									else
									{
										iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
										iValue += 150;
									}
								}
								else
								{
									//full value for permanent solutions
									iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
									iValue += 200;
								}

								if (eBestBuild != NO_BUILD)
								{
									if ((GC.getBuildInfo(eBestBuild).getImprovement() == NO_IMPROVEMENT) || !(GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
									{
										//Always prefer improvements which connect bonuses.
										eBestBuild = NO_BUILD;
										iBestValue = 0;
									}
								}
							}
							else
							{
								if (eBestBuild != NO_BUILD)
								{
									if ((GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT) && (GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
									{
										iValue -= 1000;
									}
								}
								if (GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
								{
									//very small incentive to build improvements that could later connect the bonus resource
									iValue++;
								}
							}
						}
						else if (!(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
						{
							iValue -= 1000;
						}
/*************************************************************************************************/
/**	Forts Connect Resources but should never be built					END						**/
/*************************************************************************************************/
					}
				}
				else
				{
					for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (GC.getImprovementInfo(eFinalImprovement).getImprovementBonusDiscoverRand(iJ) > 0)
						{
							iValue++;
						}
					}
				}
				
				if (iValue >= 0)
				{

                    iValue *= 2;
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/06/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
/* original BTS code
						aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);							
						}
						aiDiffYields[iJ] = (aiFinalYields[iJ] - (2 * pPlot->getYield(((YieldTypes)iJ))));
*/
						// 
						aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);							
						}

						int iCurYield = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), false));

/********************************************************************************/
/*	Better Evaluation							19.03.2010		Fuyu		    */
/********************************************************************************/
/* Fuyu: moved FinalImprovement code up to before the loop
						ImprovementTypes eCurImprovement = pPlot->getImprovementType();
*/
						if( eCurImprovement != NO_IMPROVEMENT )
						{
/* moved
							ImprovementTypes eCurFinalImprovement = finalImprovementUpgrade(eCurImprovement);
							if (eCurFinalImprovement == NO_IMPROVEMENT)
							{
								eCurFinalImprovement = eCurImprovement;
							}
*/

							iCurYield += (pPlot->calculateImprovementYieldChange(eCurFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
							iCurYield += (pPlot->calculateImprovementYieldChange(eCurImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						}
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/

						aiDiffYields[iJ] = (aiFinalYields[iJ] - iCurYield);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					}

					iValue += (aiDiffYields[YIELD_FOOD] * ((100 * iFoodPriority) / 100));
					iValue += (aiDiffYields[YIELD_PRODUCTION] * ((60 * iProductionPriority) / 100));
					iValue += (aiDiffYields[YIELD_COMMERCE] * ((40 * iCommercePriority) / 100));

					iValue /= 2;

                    for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
					    aiFinalYields[iJ] /= 2;
					    aiDiffYields[iJ] /= 2;
					}
					
					if (iValue > 0)
					{
						// this is mainly to make it improve better tiles first
						//flood plain > grassland > plain > tundra
						iValue += (aiFinalYields[YIELD_FOOD] * 10);
						iValue += (aiFinalYields[YIELD_PRODUCTION] * 6);
						iValue += (aiFinalYields[YIELD_COMMERCE] * 4);

						if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
						{
							//this is a food yielding tile
							if (iFoodPriority > 100)
							{
								iValue *= 100 + iFoodPriority;
								iValue /= 200;							
							}
							if (iFoodChange > 0)
							{
								iValue += (10 * (1 + aiDiffYields[YIELD_FOOD]) * (1 + aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION()) * iFoodChange * iFoodPriority) / 100;
							}
							if (iCommercePriority > 100)
							{
								iValue *= 100 + (((iCommercePriority - 100) * aiDiffYields[YIELD_COMMERCE]) / 2);
								iValue /= 100;
							}
						}
						else if (aiFinalYields[YIELD_FOOD] < GC.getFOOD_CONSUMPTION_PER_POPULATION())
						{
							if ((aiDiffYields[YIELD_PRODUCTION] > 0) && (aiFinalYields[YIELD_FOOD]+aiFinalYields[YIELD_PRODUCTION] > 3))
							{
								if (iFoodPriority < 100 || GET_PLAYER(getOwnerINLINE()).getCurrentEra() < 2)
								{
									//value booster for mines on hills
									iValue *= (100 + 25 * aiDiffYields[YIELD_PRODUCTION]);
									iValue /= 100;
								}
							}
							if (iFoodChange < 0)
							{
								iValue *= 4 - iFoodChange;
								iValue /= 3 + aiFinalYields[YIELD_FOOD];
							}
						}

						if ((iFoodPriority < 100) && (iProductionPriority > 100))
						{
							iValue *= (200 + ((iProductionPriority - 100)*aiFinalYields[YIELD_PRODUCTION]));
							iValue /= 200;
						}
						if (eBonus == NO_BONUS)
						{
							if (iFoodChange > 0)
							{
								//We want more food.
								iValue *= 2 + std::max(0, aiDiffYields[YIELD_FOOD]);
								iValue /= 2 * (1 + std::max(0, -aiDiffYields[YIELD_FOOD]));
							}
//							else if (iFoodChange < 0)
//							{
//								//We want to soak up food.
//								iValue *= 8;
//								iValue /= 8 + std::max(0, aiDiffYields[YIELD_FOOD]);
//							}
						}
					}
					

					if (bEmphasizeIrrigation && GC.getImprovementInfo(eFinalImprovement).isCarriesIrrigation())
					{
						iValue += 500;
					}

					if (getImprovementFreeSpecialists(eFinalImprovement) > 0)
					{
						iValue += 2000;
					}
/********************************************************************************/
/*	Better Evaluation							19.03.2010		Fuyu		    */
/********************************************************************************/
					//int iNewHappiness = GC.getImprovementInfo(eFinalImprovement).getHappiness();
					//int iCurHappiness = GC.getImprovementInfo(eCurFinalImprovement).getHappiness();
					//int iHappinessDiff = iNewHappiness - iCurHappiness;

					int iHappinessDiff = GC.getImprovementInfo(eFinalImprovement).getHappiness() - ((eCurFinalImprovement != NO_IMPROVEMENT)? GC.getImprovementInfo(eCurFinalImprovement).getHappiness() : 0);
					if ((iHappinessDiff != 0) && !(GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0))
					{
						bool bIsNegative = false;
						if (iHappinessDiff < 0)
						{
							bIsNegative = true;
							iHappinessDiff = -iHappinessDiff;
						}

						int iHappyLevel = iHappyAdjust + (happyLevel() - unhappyLevel(0)) + getEspionageHappinessCounter();
						int iHealthLevel = std::max(0, iHealthAdjust) + (goodHealth() - badHealth(false, 0)) + getEspionageHealthCounter();
					
						for (iJ = 0; iJ < iHappinessDiff; iJ++)
						{
							int iHappyValue = 0;

							if (bIsNegative)
							{
								iHappyLevel--;
							}

							if (iHappyLevel <= 0)
							{
								iHappyValue += 400;
							}
							bool bCanGrow = true;// (getYieldRate(YIELD_FOOD) > foodConsumption());
						
							if (iHappyLevel <= iHealthLevel)
							{
								iHappyValue += 200 * std::max(0, (bCanGrow ? std::min(6, 2 + iHealthLevel - iHappyLevel) : 0) - iHappyLevel);
							}
							else
							{
								iHappyValue += 50 * std::max(0, (bCanGrow ? 4 : 0) - iHappyLevel);

							}
							if (!pPlot->isBeingWorked() || bIsNegative)
							{
								iHappyValue *= 4;
								iHappyValue /= 3;
							}

							if (iHappyValue == 0 && bIsNegative && iHappyLevel < 8)
							{
								//an unhappy improvement always has negative value, even if the city is really happy right now
								iHappyValue = 10;
							}

							//iHappyValue += std::max(0, (pPlot->getCityRadiusCount() - 1)) * ((iHappyValue > 0) ? iHappyLevel / 2 : 200);
							iHappyValue += (pPlot->getPlayerCityRadiusCount(getOwnerINLINE()) - 1) * ((iHappyValue > 0) ? 200 : 50);


							if (!bIsNegative)
							{
								iHappyLevel++;
								iValue += iHappyValue;
							}
							else
							{
								iValue -= iHappyValue;
							}
						}
					}
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/

					if (!isHuman())
					{
						iValue *= std::max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getImprovementWeightModifier(eFinalImprovement) + 200));
						iValue /= 200;
					}

					if (pPlot->getImprovementType() == NO_IMPROVEMENT)
					{
						if (pPlot->isBeingWorked())
						{
							iValue *= 5;
							iValue /= 4;
						}

						if (eBestTempBuild != NO_BUILD)
						{
							if (pPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
								{
									// BBAI TODO: extra boost in early game to chop rush
									iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity) * 2;
									FAssert(pCity == this);

									iValue += iClearFeatureValue;
								}
							}
						}
					}
					else
					{
						// cottage/villages (don't want to chop them up if turns have been invested)
						ImprovementTypes eImprovementDowngrade = (ImprovementTypes)GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage();
						while (eImprovementDowngrade != NO_IMPROVEMENT)
						{
							CvImprovementInfo& kImprovementDowngrade = GC.getImprovementInfo(eImprovementDowngrade);
/********************************************************************************/
/*	Better Evaluation							19.03.2010		Fuyu		    */
/********************************************************************************/
//An attempt to make replacing hamlet+ less interesting
/* original code
							iValue -= kImprovementDowngrade.getUpgradeTime() * 8;
*/
							iValue -= kImprovementDowngrade.getUpgradeTime() * ((pPlot->isBeingWorked())? 20 : 10);
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/
							eImprovementDowngrade = (ImprovementTypes)kImprovementDowngrade.getImprovementPillage();
						}

						if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
						{
							iValue -= (GC.getImprovementInfo(pPlot->getImprovementType()).getUpgradeTime() * 8 * (pPlot->getUpgradeProgress())) / std::max(1, GC.getGameINLINE().getImprovementUpgradeTime(pPlot->getImprovementType()));
						}

						if (eNonObsoleteBonus == NO_BONUS)
						{
							if (isWorkingPlot(pPlot))
							{
								if (((iFoodPriority < 100) && (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())) || (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage() != NO_IMPROVEMENT))
								{
									iValue -= 70;
									iValue *= 2;
									iValue /= 3;
								}
							}
						}
						
						if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
						{
							iValue /= 4;	//Greatly prefer builds which are legal.
						}
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestBuild = eBestTempBuild;
						
						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiBestDiffYields[iJ] = aiDiffYields[iJ];
						}
					}
				}
			}
		}

	if (iClearFeatureValue > 0)
	{
		FAssert(pPlot->getFeatureType() != NO_FEATURE);
		
		{
			if ((GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() < 0) ||
				((GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) + GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_COMMERCE)) < 0))
			{
				for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
				{
					eBuild = ((BuildTypes)iI);

					if (GC.getBuildInfo(eBuild).getImprovement() == NO_IMPROVEMENT)
					{
						if (GC.getBuildInfo(eBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild))
							{
								iValue = iClearFeatureValue;
								iValue += (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity) * 10);
								
								iValue *= 400;
                                iValue /= std::max(1, (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 100));
                                
								if ((iValue > iBestValue) || ((iValue > 0) && (eBestBuild == NO_BUILD)))
								{
									iBestValue = iValue;
									eBestBuild = eBuild;
								}
							}
						}
					}
				}
			}
		}
	}

	//Chop - maybe integrate this better with the other feature-clear code tho the logic
	//is kinda different
    if (bChop && (eBonus == NO_BONUS) && (pPlot->getFeatureType() != NO_FEATURE) &&
        (pPlot->getImprovementType() == NO_IMPROVEMENT) && !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS)))
    {
        for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
        {
            eBuild = ((BuildTypes)iI);
            if (GC.getBuildInfo(eBuild).getImprovement() == NO_IMPROVEMENT)
            {
                if (GC.getBuildInfo(eBuild).isFeatureRemove(pPlot->getFeatureType()))
                {
                    if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild))
                    {
                        iValue = (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity)) * 10;
                        FAssert(pCity == this);

                        if (iValue > 0)
                        {
							iValue += iClearFeatureValue;

                            if (iValue > 0)
                            {
								if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_DAGGER))
								{
									iValue += 20;
									iValue *= 2;
								}
                                iValue *= 500;
                                iValue /= std::max(1, (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 100));

                                if (iValue > iBestValue)
                                {
                                    iBestValue = iValue;
                                    eBestBuild = eBuild;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
    {
        RouteTypes eRoute = (RouteTypes)iI;
        RouteTypes eOldRoute = pPlot->getRouteType();

        if (eRoute != eOldRoute)
        {
        	int iTempValue = 0;
            if (pPlot->getImprovementType() != NO_IMPROVEMENT)
            {
                if ((eOldRoute == NO_ROUTE) || (GC.getRouteInfo(eRoute).getValue() > GC.getRouteInfo(eOldRoute).getValue()))
                {
                    iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_FOOD)) * 100);
                    iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_PRODUCTION)) * 60);
                    iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_COMMERCE)) * 40);
                }
                
                if (pPlot->isBeingWorked())
                {
                	iTempValue *= 2;
                }
				//road up bonuses if sort of bored.
				if ((eOldRoute == NO_ROUTE) && (eBonus != NO_BONUS))
				{
					iTempValue += (pPlot->isConnectedToCapital() ? 10 : 30);
				}
            }
                                
			if (iTempValue > 0)
			{
				for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
				{
					eBuild = ((BuildTypes)iJ);
					if (GC.getBuildInfo(eBuild).getRoute() == eRoute)
					{
						if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
						{
							//the value multiplier is based on the default time...
							iValue = iTempValue * 5 * 300;
							iValue /= GC.getBuildInfo(eBuild).getTime();

							if ((iValue > iBestValue) || ((iValue > 0) && (eBestBuild == NO_BUILD)))
							{
								iBestValue = iValue;
								eBestBuild = eBuild;
							}
						}
					}
				}
			}
        }
    }



	if (eBestBuild != NO_BUILD)
	{
		FAssertMsg(iBestValue > 0, "iBestValue is expected to be greater than 0");
		
		//Now modify the priority for this build.
		if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
		{
			if (GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT)
			{
				iBestValue += (iBestValue * std::max(0, aiBestDiffYields[YIELD_COMMERCE])) / 4;
				iBestValue = std::max(1, iBestValue);
			}
			
		}

		if (piBestValue != NULL)
		{
			*piBestValue = iBestValue;
		}
		if (peBestBuild != NULL)
		{
			*peBestBuild = eBestBuild;
		}
	}
}


int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry)
{
	return AI_getHappyFromHurry(hurryPopulation(eHurry));
}

int CvCityAI::AI_getHappyFromHurry(int iHurryPopulation)
{
	int iHappyDiff = iHurryPopulation - GC.getDefineINT("HURRY_POP_ANGER");
	if (iHappyDiff > 0)
	{
		if (getHurryAngerTimer() <= 1)
		{
			if (2 * angryPopulation(1) - healthRate(false, 1) > 1)
			{
				return iHappyDiff;
			}
		}
	}

	return 0;
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew)
{
	return AI_getHappyFromHurry(getHurryPopulation(eHurry, getHurryCost(true, eUnit, bIgnoreNew)));
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew)
{
	return AI_getHappyFromHurry(getHurryPopulation(eHurry, getHurryCost(true, eBuilding, bIgnoreNew)));
}


int CvCityAI::AI_cityValue() const
{
	
	AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
    if ((eAreaAI == AREAAI_OFFENSIVE) || (eAreaAI == AREAAI_MASSING) || (eAreaAI == AREAAI_DEFENSIVE))
    {
        return 0;
    }
    
	int iValue = 0;
	
	iValue += getCommerceRateTimes100(COMMERCE_GOLD);
	iValue += getCommerceRateTimes100(COMMERCE_RESEARCH);
	iValue += 100 * getYieldRate(YIELD_PRODUCTION);

	iValue -= 3 * calculateColonyMaintenanceTimes100();

	return iValue;
}

bool CvCityAI::AI_doPanic()
{
	
	bool bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	
	if (bLandWar)
	{
		int iOurDefense = GET_PLAYER(getOwnerINLINE()).AI_getOurPlotStrength(plot(), 0, true, false);
		int iEnemyOffense = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(), 2, false, false);
		int iRatio = (100 * iEnemyOffense) / (std::max(1, iOurDefense));

		if (iRatio > 100)
		{
			UnitTypes eProductionUnit = getProductionUnit();

			if (eProductionUnit != NO_UNIT)
			{
				if (getProduction() > 0)
				{
					if (GC.getUnitInfo(eProductionUnit).getCombat() > 0)
					{
						AI_doHurry(true);
						return true;
					}
				}
			}
			else
			{
				if ((GC.getGame().getSorenRandNum(2, "AI choose panic unit") == 0) && AI_chooseUnit(UNITAI_CITY_COUNTER))
				{
					AI_doHurry((iRatio > 140));	
				}
				else if (AI_chooseUnit(UNITAI_CITY_DEFENSE))
				{
					AI_doHurry((iRatio > 140));
				}
				else if (AI_chooseUnit(UNITAI_ATTACK))
				{
					AI_doHurry((iRatio > 140));
				}
			}
		}
	}
	return false;
}

int CvCityAI::AI_calculateCulturePressure(bool bGreatWork)
{
    CvPlot* pLoopPlot;
    BonusTypes eNonObsoleteBonus;
    int iValue;
    int iTempValue;
    int iI;

    iValue = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
		if (pLoopPlot != NULL)
		{
		    if (pLoopPlot->getOwnerINLINE() == NO_PLAYER)
		    {
		        iValue++;
		    }
		    else
		    {
                iTempValue = pLoopPlot->calculateCulturePercent(getOwnerINLINE());
                if (iTempValue == 100)
                {
                    //do nothing
                }
                else if ((iTempValue == 0) || (iTempValue > 75))
                {
                    iValue++;
                }
                else
                {
                    iTempValue = (100 - iTempValue);
                    FAssert(iTempValue > 0);
                    FAssert(iTempValue <= 100);

                    if (iI != CITY_HOME_PLOT)
                    {
                        iTempValue *= 4;
                        iTempValue /= NUM_CITY_PLOTS;
                    }

                    eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

                    if (eNonObsoleteBonus != NO_BONUS)
                    {
                        iTempValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * ((GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses(eNonObsoleteBonus) == 0) ? 4 : 2));
                    }
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       03/20/10                          denev & jdog5000    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
					if ((iTempValue > 80) && (pLoopPlot->getOwnerINLINE() == getID()))
*/
					if ((iTempValue > 80) && (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
                    {
                        //captured territory special case
                        iTempValue *= (100 - iTempValue);
                        iTempValue /= 100;
                    }

                    if (pLoopPlot->getTeam() == getTeam())
                    {
                        iTempValue /= (bGreatWork ? 10 : 2);
                    }
                    else
                    {
                        iTempValue *= 2;
                        if (bGreatWork)
                        {
                            if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude(pLoopPlot->getOwnerINLINE()) == ATTITUDE_FRIENDLY)
                            {
                                iValue /= 10;
                            }
                        }
                    }

                    iValue += iTempValue;
                }
            }
		}
    }


    return iValue;
}


void CvCityAI::AI_buildGovernorChooseProduction()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	bool bWasFoodProduction;
	bool bDanger;
	int iCulturePressure;

	bDanger = AI_isDanger();


	// only clear the dirty bit if we actually do a check, multiple items might be queued
	AI_setChooseProductionDirty(false);

	pWaterArea = waterArea();

	bWasFoodProduction = isFoodProduction();
	iCulturePressure = AI_calculateCulturePressure();
	int iMinValueDivisor = 1;
	if (getPopulation() < 3)
	{
		iMinValueDivisor = 3;
	}
	else if (getPopulation() < 7)
	{
		iMinValueDivisor = 2;
	}
	
	
	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}
	
    // if we need to pop borders, then do that immediately if we have drama and can do it
	if ((getCultureLevel() <= (CultureLevelTypes)1) && ((getCommerceRate(COMMERCE_CULTURE) < 2) || (iCulturePressure > 0)))
	{
        if (AI_chooseProcess(COMMERCE_CULTURE))
        {
            return;
        }
	}
	
// BUG - Governor Builds Workboats - start
#ifdef _MOD_GOVWORKERS
	if (!isHuman() || GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_MODDER_1))
	{
#endif
	//workboat
	if (pWaterArea != NULL)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_WORKER_SEA) == 0)
		{
			if (AI_neededSeaWorkers() > 0)
			{
				if (AI_chooseUnit(UNITAI_WORKER_SEA))
				{
					return;
				}
			}
		}
	}
#ifdef _MOD_GOVWORKERS
	}
#endif
// BUG - Governor Builds Workboats - end

	if ((AI_countNumBonuses(NO_BONUS, false, true, 10, true, true) > 0)
		&& (getPopulation() > AI_countNumBonuses(NO_BONUS, true, false, -1, true, true)))
	{
		if (getCommerceRate(COMMERCE_CULTURE) == 0)
		{
			AI_chooseBuilding(BUILDINGFOCUS_CULTURE);
			return;
		}
	}

    // pick granary or lighthouse, any duration
    if (AI_chooseBuilding(BUILDINGFOCUS_FOOD))
    {
        return;
    }
    
    if (angryPopulation(1) > 1)
    {
        if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, 40))
        {
            return;
        }
    }
    
	if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 30, 10 / iMinValueDivisor))
    {
        return;
    }
    
	if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 8, 33))
    {
        return;
    }

    
	if (((getCommerceRateTimes100(COMMERCE_CULTURE) == 0) && (iCulturePressure != 0))
        || (iCulturePressure > 100))
    {
        if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30))
        {
            return;
        }
    }

    
	int iEconomyFlags = 0;
	iEconomyFlags |= BUILDINGFOCUS_GOLD;
	iEconomyFlags |= BUILDINGFOCUS_RESEARCH;
	iEconomyFlags |= BUILDINGFOCUS_MAINTENANCE;
	iEconomyFlags |= BUILDINGFOCUS_HAPPY;
	iEconomyFlags |= BUILDINGFOCUS_HEALTHY;
	iEconomyFlags |= BUILDINGFOCUS_SPECIALIST;
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		iEconomyFlags |= BUILDINGFOCUS_ESPIONAGE;
	}
	
	//20 means 5g or ~2 happiness...
	if (AI_chooseBuilding(iEconomyFlags, 20, 20 / iMinValueDivisor))
	{
		return;
	}
	
// BUG - Governor Builds Workers - start
#ifdef _MOD_GOVWORKERS
	if (!isHuman() || GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_MODDER_2))
	{
#endif
	int iExistingWorkers = GET_PLAYER(getOwner()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER);
    int iNeededWorkers = GET_PLAYER(getOwner()).AI_neededWorkers(area());
    
	if (!bDanger && (iExistingWorkers < ((iNeededWorkers + 1) / 2)))
	{
		if (AI_chooseUnit(UNITAI_WORKER))
		{
			return;
		}
	}	    
#ifdef _MOD_GOVWORKERS
	}
#endif
// BUG - Governor Builds Workers - end

    if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
    {
        if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > 0)
        {
            if (AI_chooseBuilding(BUILDINGFOCUS_SPECIALIST, 60))
            {
                return;
            }
        }
    }
    
	if (AI_chooseBuilding(iEconomyFlags, 40, 15 / iMinValueDivisor))
	{
		return;
	}
	
	if (AI_chooseBuilding(iEconomyFlags | BUILDINGFOCUS_CULTURE, 10, 10 / iMinValueDivisor))
	{
		return;
	}


	if (AI_chooseProcess())
	{
		return;
	}

	if (AI_chooseBuilding())
	{
		return;
	}
	
    if (AI_chooseUnit())
	{
		return;
	}
}

int CvCityAI::AI_calculateWaterWorldPercent()
{
    int iI;
    int iWaterPercent = 0;
    int iTeamCityCount = 0;
	int iOtherCityCount = 0;
	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI == getTeam() || GET_TEAM((TeamTypes)iI).isVassal(getTeam())
				|| GET_TEAM(getTeam()).isVassal((TeamTypes)iI))
			{
				iTeamCityCount += GET_TEAM((TeamTypes)iI).countNumCitiesByArea(area());
			}
			else
			{
				iOtherCityCount += GET_TEAM((TeamTypes)iI).countNumCitiesByArea(area());
			}
		}
	}

    if (iOtherCityCount == 0)
    {
        iWaterPercent = 100;
    }
    else
    {
        iWaterPercent = 100 - ((iTeamCityCount + iOtherCityCount) * 100) / std::max(1, (GC.getGame().getNumCities()));
    }
    
    iWaterPercent *= 50;
    iWaterPercent /= 100;
    
    iWaterPercent += (50 * (2 + iTeamCityCount)) / (2 + iTeamCityCount + iOtherCityCount);
    
    iWaterPercent = std::max(1, iWaterPercent);
    
    
    return iWaterPercent;
}

//Please note, takes the yield multiplied by 100
int CvCityAI::AI_getYieldMagicValue(const int* piYieldsTimes100, bool bHealthy)
{
	FAssert(piYieldsTimes100 != NULL);

    int iPopEats = GC.getFOOD_CONSUMPTION_PER_POPULATION();
    iPopEats += (bHealthy ? 0 : 1);
    iPopEats *= 100;

    int iValue = ((piYieldsTimes100[YIELD_FOOD] * 100 + piYieldsTimes100[YIELD_PRODUCTION]*55 + piYieldsTimes100[YIELD_COMMERCE]*40) - iPopEats * 102);
    iValue /= 100;
    return iValue;
}

//The magic value is basically "Look at this plot, is it worth working"
//-50 or lower means the plot is worthless in a "workers kill yourself" kind of way.
//-50 to -1 means the plot isn't worth growing to work - might be okay with emphasize though.
//Between 0 and 50 means it is marginal.
//50-100 means it's okay.
//Above 100 means it's definitely decent - seriously question ever not working it.
//This function deliberately doesn't use emphasize settings.
int CvCityAI::AI_getPlotMagicValue(CvPlot* pPlot, bool bHealthy, bool bWorkerOptimization)
{
    int aiYields[NUM_YIELD_TYPES];
    ImprovementTypes eCurrentImprovement;
    ImprovementTypes eFinalImprovement;
    int iI;
    int iYieldDiff;
    
    FAssert(pPlot != NULL);

    for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
    {
    	if ((bWorkerOptimization) && (pPlot->getWorkingCity() == this) && (AI_getBestBuild(getCityPlotIndex(pPlot)) != NO_BUILD))
    	{
    		aiYields[iI] = pPlot->getYieldWithBuild(AI_getBestBuild(getCityPlotIndex(pPlot)), (YieldTypes)iI, true);
    	}
    	else
    	{
        	aiYields[iI] = pPlot->getYield((YieldTypes)iI) * 100;
    	}
    }

    eCurrentImprovement = pPlot->getImprovementType();

    if (eCurrentImprovement != NO_IMPROVEMENT)
    {
        eFinalImprovement = finalImprovementUpgrade(eCurrentImprovement);

        if ((eFinalImprovement != NO_IMPROVEMENT) && (eFinalImprovement != eCurrentImprovement))
        {
            for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
            {
                iYieldDiff = 100 * pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iI), getOwnerINLINE());
                iYieldDiff -= 100 * pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes)iI), getOwnerINLINE());
                aiYields[iI] += iYieldDiff / 2;
            }
        }
    }
    
    return AI_getYieldMagicValue(aiYields, bHealthy);
}

//useful for deciding whether or not to grow... or whether the city needs terrain
//improvement.
//if healthy is false it assumes bad health conditions.
int CvCityAI::AI_countGoodTiles(bool bHealthy, bool bUnworkedOnly, int iThreshold, bool bWorkerOptimization)
{
    CvPlot* pLoopPlot;
    int iI;
    int iCount;
    
    iCount = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(), iI);
        if ((iI != CITY_HOME_PLOT) && (pLoopPlot != NULL))
        {
            if (pLoopPlot->getWorkingCity() == this)
            {
                if (!bUnworkedOnly || !(pLoopPlot->isBeingWorked()))
                {
                    if (AI_getPlotMagicValue(pLoopPlot, bHealthy) > iThreshold)
                    {
                        iCount++;
                    }                    
                }
            }
        }
    }
    return iCount;
}

int CvCityAI::AI_calculateTargetCulturePerTurn()
{
	/*
	int iTarget = 0;
	
	bool bAnyGoodPlotUnowned = false;
	bool bAnyGoodPlotHighPressure = false;	
	
	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		CvPlot* pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(),iI);
        
        if (pLoopPlot != NULL)
        {
			if ((pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				|| (pLoopPlot->getYield(YIELD_FOOD) > GC.getFOOD_CONSUMPTION_PER_POPULATION()))
			{
				if (!pLoopPlot->isOwned())
				{
					bAnyGoodPlotUnowned = true;        			
				}
				else if (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())
				{
					bAnyGoodPlotHighPressure = true;
				}
			}
        }
	}
	if (bAnyGoodPlotUnowned)
	{
		iTarget = 1;
	}
	if (bAnyGoodPlotHighPressure)
	{
		iTarget += getCommerceRate(COMMERCE_CULTURE) + 1;
	}
	return iTarget;
	*/
	return 1;
}
	
int CvCityAI::AI_countGoodSpecialists(bool bHealthy)
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iCount = 0;
	for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes)iI;
		
		int iValue = 0;
		
		iValue += 100 * kPlayer.specialistYield(eSpecialist, YIELD_FOOD);
		iValue += 65 * kPlayer.specialistYield(eSpecialist, YIELD_PRODUCTION);
		iValue += 40 * kPlayer.specialistYield(eSpecialist, YIELD_COMMERCE);
		
		iValue += 40 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_RESEARCH);
		iValue += 40 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_GOLD);
		iValue += 20 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_ESPIONAGE);
		iValue += 15 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_CULTURE);
		iValue += 25 * GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();
		
		if (iValue >= (bHealthy ? 200 : 300))
		{
			iCount += getMaxSpecialistCount(eSpecialist);
		}
	}
	iCount -= getFreeSpecialist();
	
	return iCount;
}
//0 is normal
//higher than zero means special.
int CvCityAI::AI_getCityImportance(bool bEconomy, bool bMilitary)
{
    int iValue = 0;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
    {
        int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
        int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();
        
        if (iCultureRateRank <= iCulturalVictoryNumCultureCities)
        {
            iValue += 100;
        
            if ((getCultureLevel() < (GC.getNumCultureLevelInfos() - 1)))
            {
                iValue += !bMilitary ? 100 : 0;
            }
            else
            {
                iValue += bMilitary ? 100 : 0;
            }
        }
    }
    
    return iValue;
}

void CvCityAI::AI_stealPlots()
{
    PROFILE_FUNC();
    CvPlot* pLoopPlot;
    CvCityAI* pWorkingCity;
    int iI;
    int iOtherImportance;
    
    int iImportance = AI_getCityImportance(true, false);

    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(),iI);
        
        if (pLoopPlot != NULL)
        {
            if (iImportance > 0)
            {
                if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
                {
                    pWorkingCity = static_cast<CvCityAI*>(pLoopPlot->getWorkingCity());
                    if ((pWorkingCity != this) && (pWorkingCity != NULL))
                    {
                        FAssert(pWorkingCity->getOwnerINLINE() == getOwnerINLINE());
                        iOtherImportance = pWorkingCity->AI_getCityImportance(true, false);
                        if (iImportance > iOtherImportance)
                        {
                            pLoopPlot->setWorkingCityOverride(this);
                        }
                    }
                }
            }
                
            if (pLoopPlot->getWorkingCityOverride() == this)
            {
                if (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())
                {
                    pLoopPlot->setWorkingCityOverride(NULL);                    
                }
            }
        }
    }
}




// +1/+3/+5 plot based on base food yield (1/2/3)
// +4 if being worked.
// +4 if a bonus.
// Unworked ocean ranks very lowly. Unworked lake ranks at 3. Worked lake at 7.
// Worked bonus in ocean ranks at like 11
int CvCityAI::AI_buildingSpecialYieldChangeValue(BuildingTypes eBuilding, YieldTypes eYield)
{
    int iI;
    CvPlot* pLoopPlot;
    int iValue = 0;
    CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
    int iWorkedCount = 0;
    
    int iYieldChange = kBuilding.getSeaPlotYieldChange(eYield);
    if (iYieldChange > 0)
    {
        int iWaterCount = 0;
        for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
        {
            if (iI != CITY_HOME_PLOT)
            {
                pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
                if ((pLoopPlot != NULL) && (pLoopPlot->getWorkingCity() == this))
                {
                    if (pLoopPlot->isWater())
                    {
                        iWaterCount++;
                        int iFood = pLoopPlot->getYield(YIELD_FOOD);
                        iFood += (eYield == YIELD_FOOD) ? iYieldChange : 0;
                        
                        iValue += std::max(0, iFood * 2 - 1);
                        if (pLoopPlot->isBeingWorked())
                        {
                        	iValue += 4;
                        	iWorkedCount++;
                        }
                        iValue += ((pLoopPlot->getBonusType(getTeam()) != NO_BONUS) ? 8 : 0);
                    }
                }
            }
        }
    }
    if (iWorkedCount == 0)
    {
		SpecialistTypes eDefaultSpecialist = (SpecialistTypes)GC.getDefineINT("DEFAULT_SPECIALIST");
		if ((getPopulation() > 2) && ((eDefaultSpecialist == NO_SPECIALIST) || (getSpecialistCount(eDefaultSpecialist) == 0)))
		{
			iValue /= 2;		
		}
    }

    return iValue;
}


int CvCityAI::AI_yieldMultiplier(YieldTypes eYield)
{
	PROFILE_FUNC();
	
	int iMultiplier = getBaseYieldRateModifier(eYield);
	
	if (eYield == YIELD_PRODUCTION)
	{
		iMultiplier += (getMilitaryProductionModifier() / 2);
	}
	
	if (eYield == YIELD_COMMERCE)
	{
		iMultiplier += (getCommerceRateModifier(COMMERCE_RESEARCH) * 60) / 100;
		iMultiplier += (getCommerceRateModifier(COMMERCE_GOLD) * 35) / 100;
		iMultiplier += (getCommerceRateModifier(COMMERCE_CULTURE) * 15) / 100;
	}
	
	return iMultiplier;	
}
//this should be called before doing governor stuff.
//This is the function which replaces emphasis
//Could stand for a Commerce Variety to be added
//especially now that there is Espionage
void CvCityAI::AI_updateSpecialYieldMultiplier()
{
	PROFILE_FUNC();
	
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSpecialYieldMultiplier[iI] = 0;
	}
	
	UnitTypes eProductionUnit = getProductionUnit();
	if (eProductionUnit != NO_UNIT)
	{
		if (GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_WORKER_SEA)
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 50;				
		}
		if ((GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_WORKER) ||
			(GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_SETTLE))
		
		{
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 50;			
		}
	}

	BuildingTypes eProductionBuilding = getProductionBuilding();
	if (eProductionBuilding != NO_BUILDING)
	{
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))
			|| isProductionProject())
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 25;					
		}
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += std::max(-25, GC.getBuildingInfo(eProductionBuilding).getFoodKept());
		
		if ((GC.getBuildingInfo(eProductionBuilding).getCommerceChange(COMMERCE_CULTURE) > 0)
			|| (GC.getBuildingInfo(eProductionBuilding).getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0))
		{
			int iTargetCultureRate = AI_calculateTargetCulturePerTurn();
			if (iTargetCultureRate > 0)
			{
				if (getCommerceRate(COMMERCE_CULTURE) == 0)
				{
					m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
				}
				else if (getCommerceRate(COMMERCE_CULTURE) < iTargetCultureRate)
				{
					m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 20;					
				}
			}
		}
	}
	
	// non-human production value increase
	if (!isHuman())
	{
		CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
		AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
		
		if ((kPlayer.AI_isDoStrategy(AI_STRATEGY_DAGGER) && getPopulation() >= 4)
			|| (eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) 
			|| (eAreaAIType == AREAAI_MASSING) || (eAreaAIType == AREAAI_ASSAULT))
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 10;
			if (!kPlayer.AI_isFinancialTrouble())
			{
				m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 40;
			}
		}
		
		int iIncome = 1 + kPlayer.getCommerceRate(COMMERCE_GOLD) + kPlayer.getCommerceRate(COMMERCE_RESEARCH) + std::max(0, kPlayer.getGoldPerTurn());
		int iExpenses = 1 + kPlayer.calculateInflatedCosts() - std::min(0, kPlayer.getGoldPerTurn());
		FAssert(iIncome > 0);
		
/************************************************************************************************/
/* Afforess	                  Start		 12/7/09                                                */
/*                                                                                              */
/*       Divide By Zero Fix                                                                     */
/************************************************************************************************/
		int iRatio = (100 * iExpenses) / std::max(1, iIncome);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		//Gold -> Production Reduced To
		// 40- -> 100%
		// 60 -> 83%
		// 100 -> 28%
		// 110+ -> 14%
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 100;
		if (iRatio > 60)
		{
			//Greatly decrease production weight 
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] *= std::max(10, 120 - iRatio);
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] /= 72;
		}
		else if (iRatio > 40)
		{
			//Slightly decrease production weight.
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] *= 160 - iRatio;
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] /= 120;
		}
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] -= 100;
	}
}

int CvCityAI::AI_specialYieldMultiplier(YieldTypes eYield)
{
	return m_aiSpecialYieldMultiplier[eYield];
}


int CvCityAI::AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand, bool bWater)
{
    CvPlot* pLoopPlot;
    BonusTypes eLoopBonus;
    int iI;
    int iCount = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
        
        if (pLoopPlot != NULL)
        {
        	if ((pLoopPlot->area() == area()) || (bWater && pLoopPlot->isWater()))
        	{
				eLoopBonus = pLoopPlot->getBonusType(getTeam());
				if (eLoopBonus != NO_BONUS)
				{
					if ((eBonus == NO_BONUS) || (eBonus == eLoopBonus))
					{
						if (bIncludeOurs && (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) && (pLoopPlot->getWorkingCity() == this))
						{
							iCount++;                    
						}
						else if (bIncludeNeutral && (!pLoopPlot->isOwned()))
						{
							iCount++;
						}
						else if ((iOtherCultureThreshold > 0) && (pLoopPlot->isOwned() && (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())))
						{
							if ((pLoopPlot->getCulture(pLoopPlot->getOwnerINLINE()) - pLoopPlot->getCulture(getOwnerINLINE())) < iOtherCultureThreshold)
							{
								iCount++;
							}                        
						}
					}
				}
        	}
        }
    }
    
    
    return iCount;    
    
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      11/14/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
int CvCityAI::AI_countNumImprovableBonuses( bool bIncludeNeutral, TechTypes eExtraTech, bool bLand, bool bWater )
{
	CvPlot* pLoopPlot;
    BonusTypes eLoopBonus;
    int iI;
    int iCount = 0;
    for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
    {
        pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
        
        if (pLoopPlot != NULL)
        {
        	if ((bLand && pLoopPlot->area() == area()) || (bWater && pLoopPlot->isWater()))
        	{
				eLoopBonus = pLoopPlot->getBonusType(getTeam());
				if (eLoopBonus != NO_BONUS)
				{
					if ( ((pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) && (pLoopPlot->getWorkingCity() == this)) || (bIncludeNeutral && (!pLoopPlot->isOwned())))
					{
						for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
						{
							BuildTypes eBuild = ((BuildTypes)iJ);
							
							if( eBuild != NO_BUILD && pLoopPlot->canBuild(eBuild, getOwnerINLINE()) )
							{
								ImprovementTypes eImp = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();

								if( eImp != NO_IMPROVEMENT && GC.getImprovementInfo(eImp).isImprovementBonusTrade(eLoopBonus) )
								{
									if( GET_PLAYER(getOwnerINLINE()).canBuild(pLoopPlot, eBuild) )
									{
										iCount++;
										break;
									}
									else if( (eExtraTech != NO_TECH) )
									{
										if (GC.getBuildInfo(eBuild).getTechPrereq() == eExtraTech)
										{
											iCount++;
											break;
										}
									}
								}
							}
						}
					}
				}
        	}
        }
    }
    
    
    return iCount;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

int CvCityAI::AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance)
{
	FAssert(GET_PLAYER(eIndex).isAlive());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/16/10                              jdog5000        */
/*                                                                                              */
/* War tactics AI                                                                               */
/************************************************************************************************/
/* original bts code
	FAssert(eIndex != getID());
*/
	// No point checking player type against city ID ... Firaxis copy and paste error from
	// CvPlayerAI version of this function
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	if ((m_iCachePlayerClosenessTurn != GC.getGame().getGameTurn())
		|| (m_iCachePlayerClosenessDistance != iMaxDistance))
	{
		AI_cachePlayerCloseness(iMaxDistance);
	}
	
	return m_aiPlayerCloseness[eIndex];	
}

void CvCityAI::AI_cachePlayerCloseness(int iMaxDistance)
{
	PROFILE_FUNC();
	CvCity* pLoopCity;
	int iI;
	int iLoop;
	int iValue;
	int iTempValue;
	int iBestValue;

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						5/16/10				jdog5000		*/
/* 																				*/
/* 	General AI, closeness changes												*/
/********************************************************************************/	
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && 
			((GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))))
		{
			iValue = 0;
			iBestValue = 0;
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if( pLoopCity == this )
				{
					continue;
				}

				int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
				
				if (area() != pLoopCity->area() )
				{
					iDistance += 1;
					iDistance /= 2;
				}
				if (iDistance <= iMaxDistance)
				{
					if ( getArea() == pLoopCity->getArea() )
					{
						int iPathDistance = GC.getMap().calculatePathDistance(plot(), pLoopCity->plot());
						if (iPathDistance > 0)
						{
							iDistance = iPathDistance;
						}
					}
					if (iDistance <= iMaxDistance)
					{
						// Weight by population of both cities, not just pop of other city
						//iTempValue = 20 + 2*pLoopCity->getPopulation();
						iTempValue = 20 + pLoopCity->getPopulation() + getPopulation();

						iTempValue *= (1 + (iMaxDistance - iDistance));
						iTempValue /= (1 + iMaxDistance);
						
						//reduce for small islands.
						int iAreaCityCount = pLoopCity->area()->getNumCities();
						iTempValue *= std::min(iAreaCityCount, 5);
						iTempValue /= 5;
						if (iAreaCityCount < 3)
						{
							iTempValue /= 2;
						}
						
						if (pLoopCity->isBarbarian())
						{
							iTempValue /= 4;
						}
						
						iValue += iTempValue;					
						iBestValue = std::max(iBestValue, iTempValue);
					}
				}
			}
			m_aiPlayerCloseness[iI] = (iBestValue + iValue / 4);
		}
	}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END							        */
/********************************************************************************/
	
	m_iCachePlayerClosenessTurn = GC.getGame().getGameTurn();	
	m_iCachePlayerClosenessDistance = iMaxDistance;
}

int CvCityAI::AI_cityThreat(bool bDangerPercent)
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/04/10                                jdog5000      */
/*                                                                                              */
/* War tactics AI                                                                               */
/************************************************************************************************/
	PROFILE_FUNC();
	int iValue = 0;
	bool bCrushStrategy = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CRUSH);

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if ((iI != getOwner()) && GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			int iTempValue = AI_playerCloseness((PlayerTypes)iI, DEFAULT_PLAYER_CLOSENESS);
			if (iTempValue > 0)
			{
				if ((bCrushStrategy) && (GET_TEAM(getTeam()).AI_getWarPlan(GET_PLAYER((PlayerTypes)iI).getTeam()) != NO_WARPLAN))
				{
					iTempValue *= 400;						
				}
				else if (atWar(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					iTempValue *= 300;
				}
				// Beef up border security before starting war, but not too much
				else if ( GET_TEAM(getTeam()).AI_getWarPlan(GET_PLAYER((PlayerTypes)iI).getTeam()) != NO_WARPLAN )
				{
					iTempValue *= 180;
				}
				// Extra trust of/for Vassals, regardless of relations
				else if ( GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()) ||
							GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					iTempValue *= 30;
				}
				else
				{
					switch (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI))
					{
					case ATTITUDE_FURIOUS:
						iTempValue *= 180;
						break;

					case ATTITUDE_ANNOYED:
						iTempValue *= 130;
						break;

					case ATTITUDE_CAUTIOUS:
						iTempValue *= 100;
						break;

					case ATTITUDE_PLEASED:
						iTempValue *= 50;
						break;

					case ATTITUDE_FRIENDLY:
						iTempValue *= 20;
						break;

					default:
						FAssert(false);
						break;
					}

					// Beef up border security next to powerful rival, (Fuyu) just not too much if our units are weaker on average
					if( GET_PLAYER((PlayerTypes)iI).getPower() > GET_PLAYER(getOwnerINLINE()).getPower() )
					{
						int iTempMultiplier = std::min( 400, (100 * GET_PLAYER((PlayerTypes)iI).getPower())/std::max(1, GET_PLAYER(getOwnerINLINE()).getPower()) );
						iTempMultiplier += range(( (100 * GET_PLAYER((PlayerTypes)iI).getNumMilitaryUnits())/std::max(1, GET_PLAYER(getOwnerINLINE()).getNumMilitaryUnits()) ), 100, iTempMultiplier);
						iTempMultiplier /= 2;
						iTempValue *= iTempMultiplier;
						iTempValue /= 100;
					}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       01/04/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
					if (bCrushStrategy)
					{
						iValue /= 2;
					}
*/
					if (bCrushStrategy)
					{
						iTempValue /= 2;
					}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
				}
				iTempValue /= 100;
				iValue += iTempValue;
			}
		}
	}
	
	if (isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
	{
		int iCurrentEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
		iValue += std::max(0, ((10 * iCurrentEra) / 3) - 6); //there are better ways to do this
	}
	
	iValue += getNumActiveWorldWonders() * 5;

	if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
	{
		iValue += 5;
		iValue += getCommerceRateModifier(COMMERCE_CULTURE) / 20;
		if (getCultureLevel() >= (GC.getNumCultureLevelInfos() - 2))
		{
			iValue += 20;
			if (getCultureLevel() >= (GC.getNumCultureLevelInfos() - 1))
			{
				iValue += 30;
			}
		}
	}
	
	iValue += 2 * GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3, false);
	
	return iValue;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}

//Workers have/needed is not intended to be a strict
//target but rather an indication.
//if needed is at least 1 that means a worker
//will be doing something useful
int CvCityAI::AI_getWorkersHave()
{
	return m_iWorkersHave;	
}

int CvCityAI::AI_getWorkersNeeded()
{
	return m_iWorkersNeeded;
}

void CvCityAI::AI_changeWorkersHave(int iChange)
{
	m_iWorkersHave += iChange;
	//FAssert(m_iWorkersHave >= 0);
	m_iWorkersHave = std::max(0, m_iWorkersHave);
}
	
//This needs to be serialized for human workers.
void CvCityAI::AI_updateWorkersNeededHere()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	
	short aiYields[NUM_YIELD_TYPES];
	
	int iWorkersNeeded = 0;
	int iWorkersHave = 0;
	int iUnimprovedWorkedPlotCount = 0;
	int iUnimprovedUnworkedPlotCount = 0;
	int iWorkedUnimprovableCount = 0;
	int iImprovedUnworkedPlotCount = 0;
	
	int iSpecialCount = 0;
	
	int iWorstWorkedPlotValue = MAX_INT;
	int iBestUnworkedPlotValue = 0;
	
	iWorkersHave = 0;
	
	if (getProductionUnit() != NO_UNIT)
	{
		if (getProductionUnitAI() == UNITAI_WORKER)
		{
			if (getProductionTurnsLeft() <= 2)
			{
				iWorkersHave++;
			}
		}
	}

/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
	//iWorkersHave += iWorkersHaveByPlotTargetMissionAI;
	int iWorkersHaveByPlotTargetMissionAI = AI_workingCityPlotTargetMissionAIs(getOwnerINLINE(), MISSIONAI_BUILD, UNITAI_WORKER, true);
	int iWorkersHaveNewlyBuilt = 0;
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
		{
			if (pLoopPlot->getArea() == getArea())
			{
				// BBAI TODO: Check late game slowness 

				//How slow is this? It could be almost NUM_CITY_PLOT times faster
				//by iterating groups and seeing if the plot target lands in this city
				//but since this is only called once/turn i'm not sure it matters.

/********************************************************************************/
/* 	Worker Counting							03.08.2010			Fuyu			*/
/********************************************************************************/
//Fuyu: it might matter, so if we can make it NUM_CITY_PLOT times faster
//then lets do it. Moved to before the loop
/* original code
				iWorkersHave += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));
*/

//was counting workers twice because missionais are never reset
/* original code
				iWorkersHave += pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwner(), getTeam(), PUF_isNoMission, -1, -1);
*/
				//iWorkersHave += pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwner(), getTeam(), PUF_isNoMissionAI, -1, -1);
				iWorkersHaveNewlyBuilt += pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwner(), getTeam(), PUF_isNoMissionAI, -1, -1);
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

				if (iI != CITY_HOME_PLOT)
				{
					if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
					{
						if (pLoopPlot->isBeingWorked())
						{
							if (AI_getBestBuild(iI) != NO_BUILD)
							{
								iUnimprovedWorkedPlotCount++;
							}
							else
							{
								iWorkedUnimprovableCount++;
							}
						}
						else
						{
							if (AI_getBestBuild(iI) != NO_BUILD)
							{
								iUnimprovedUnworkedPlotCount++;
							}
						}
					}
					else
					{
						if (!pLoopPlot->isBeingWorked())
						{
							iImprovedUnworkedPlotCount++;
						}
					}

					for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
					}

					if (pLoopPlot->isBeingWorked())
					{
						int iPlotValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						iWorstWorkedPlotValue = std::min(iWorstWorkedPlotValue, iPlotValue);					
					}
					else
					{
						int iPlotValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						iBestUnworkedPlotValue = std::max(iBestUnworkedPlotValue, iPlotValue);
					}
				}
			}
		}
	}
	//specialists?
	
	iUnimprovedWorkedPlotCount += std::min(iUnimprovedUnworkedPlotCount, iWorkedUnimprovableCount) / 2;
	
	iWorkersNeeded += 2 * iUnimprovedWorkedPlotCount;
	
	int iBestPotentialPlotValue = -1;
	if (iWorstWorkedPlotValue != MAX_INT)
	{
		//Add an additional citizen to account for future growth.
		int iBestPlot = -1;
		SpecialistTypes eBestSpecialist = NO_SPECIALIST;
		
		if (angryPopulation() == 0)
		{
			AI_addBestCitizen(true, true, &iBestPlot, &eBestSpecialist);
		}
		
		for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this && pLoopPlot->getArea() == getArea())
				{
					if (AI_getBestBuild(iI) != NO_BUILD)
					{
						for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiYields[iJ] = pLoopPlot->getYieldWithBuild(m_aeBestBuild[iI], (YieldTypes)iJ, true);
						}
						
						int iPlotValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(AI_getBestBuild(iI)).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							if ((getImprovementFreeSpecialists(eImprovement) > 0) || (GC.getImprovementInfo(eImprovement).getHappiness() > 0))
							{
								iSpecialCount ++;
							}
						}
						iBestPotentialPlotValue = std::max(iBestPotentialPlotValue, iPlotValue);
					}
				}
			}
		}
		
		if (iBestPlot != -1)
		{
			setWorkingPlot(iBestPlot, false);
		}
		if (eBestSpecialist != NO_SPECIALIST)
		{
			changeSpecialistCount(eBestSpecialist, -1);
		}
		
		if (iBestPotentialPlotValue > iWorstWorkedPlotValue)
		{
			iWorkersNeeded += 2;			
		}
	}
	
	iWorkersNeeded += (std::max(0, iUnimprovedWorkedPlotCount - 1) * (GET_PLAYER(getOwnerINLINE()).getCurrentEra())) / 3;
	
	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iWorkersNeeded *= 3;
		iWorkersNeeded /= 2;
	}
	
	if (iWorkersNeeded > 0)
	{
		iWorkersNeeded++;
		iWorkersNeeded = std::max(1, iWorkersNeeded / 3);
	}
	
	int iSpecialistExtra = std::min((getSpecialistPopulation() - totalFreeSpecialists()), iUnimprovedUnworkedPlotCount);
	iSpecialistExtra -= iImprovedUnworkedPlotCount;
	
	iWorkersNeeded += std::max(0, 1 + iSpecialistExtra) / 2;
	
	if (iWorstWorkedPlotValue <= iBestUnworkedPlotValue && iBestUnworkedPlotValue >= iBestPotentialPlotValue)
	{
		iWorkersNeeded /= 2;
	}
	if (angryPopulation(1) > 0)
	{
		iWorkersNeeded++;
		iWorkersNeeded /= 2;
	}
	
	iWorkersNeeded += (iSpecialCount + 1) / 2;
	
	iWorkersNeeded = std::max((iUnimprovedWorkedPlotCount + 1) / 2, iWorkersNeeded);
/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
	//if( gCityLogLevel >= 3 ) logBBAI("      City %S has %d workers: %d from plotTarget, %d newly built, %d finished soon", getName().GetCString(), (iWorkersHave + iWorkersHaveNewlyBuilt + iWorkersHaveByPlotTargetMissionAI), iWorkersHaveByPlotTargetMissionAI, iWorkersHaveNewlyBuilt, iWorkersHave);
	iWorkersHave += iWorkersHaveNewlyBuilt;
	iWorkersHave += iWorkersHaveByPlotTargetMissionAI;
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

	m_iWorkersNeeded = iWorkersNeeded;
	m_iWorkersHave = iWorkersHave;
}
/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
int CvCityAI::AI_workingCityPlotTargetMissionAIs(PlayerTypes ePlayer, MissionAITypes eMissionAI, UnitAITypes eUnitAI, bool bSameAreaOnly) const
{
	PROFILE_FUNC();

	if (this == NULL)
	{
		return 0;
	}

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	bool bCanMoveAllTerrain = bSameAreaOnly; //only check if bSameAreaOnly
	int iCorrectUnitAICount;
	int iCount = 0;

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = kPlayer.firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = kPlayer.nextSelectionGroup(&iLoop))
	{
		CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

		if (pMissionPlot != NULL)
		{
			if (eMissionAI == NO_MISSIONAI || pLoopSelectionGroup->AI_getMissionAIType() == eMissionAI)
			{
				if (pMissionPlot->getWorkingCity() == this)
				{
					if (eUnitAI == NO_UNITAI && !bSameAreaOnly)
					{
						iCount += pLoopSelectionGroup->getNumUnits();
					}
					else
					{
						CLLNode<IDInfo>* pUnitNode;
						CvUnit* pLoopUnit;

						pUnitNode = pLoopSelectionGroup->headUnitNode();
						if (pUnitNode != NULL)
						{
							CvUnit* pHeadUnit = ::getUnit(pUnitNode->m_data);
							iCorrectUnitAICount = 0;

							if (pHeadUnit != NULL)
							{
								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pLoopSelectionGroup->nextUnitNode(pUnitNode);

									if (bCanMoveAllTerrain && !(pLoopUnit->canMoveAllTerrain()))
									{
										bCanMoveAllTerrain = false;
									}
									if (eUnitAI == NO_UNITAI || pLoopUnit->AI_getUnitAIType() == eUnitAI)
									{
										iCorrectUnitAICount++;
									}
								}

								if (!bSameAreaOnly || bCanMoveAllTerrain || pHeadUnit->getArea() == pMissionPlot->getArea())
								{
									iCount += iCorrectUnitAICount;
								}
							}
						}
					}
				}
			}
		}
	}

	return iCount;
}
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

BuildingTypes CvCityAI::AI_bestAdvancedStartBuilding(int iPass)
{
	int iFocusFlags = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/07/10                                               */
/************************************************************************************************/
	if (iPass >= 0)
	{
		iFocusFlags |= BUILDINGFOCUS_CULTURE;
	}
	if (iPass >= 1)
	{
		iFocusFlags |= BUILDINGFOCUS_FOOD;
	}
	if (iPass >= 2)
	{
		iFocusFlags |= BUILDINGFOCUS_PRODUCTION;
	}
	if (iPass >= 3)
	{
		iFocusFlags |= BUILDINGFOCUS_EXPERIENCE;
	}
	if (iPass >= 4)
	{
		iFocusFlags |= (BUILDINGFOCUS_HAPPY | BUILDINGFOCUS_HEALTHY);
	}
	if (iPass >= 5)
	{
		iFocusFlags |= (BUILDINGFOCUS_GOLD | BUILDINGFOCUS_RESEARCH | BUILDINGFOCUS_MAINTENANCE);
		if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
		{
			iFocusFlags |= BUILDINGFOCUS_ESPIONAGE;
		}
	}
			
	return AI_bestBuildingThreshold(iFocusFlags, 0, std::max(0, 25 - iPass * 5));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}
//
//
//
void CvCityAI::read(FDataStreamBase* pStream)
{
	CvCity::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iEmphasizeAvoidGrowthCount);
	pStream->Read(&m_iEmphasizeGreatPeopleCount);
	pStream->Read(&m_bAssignWorkDirty);
	pStream->Read(&m_bChooseProductionDirty);

	pStream->Read((int*)&m_routeToCity.eOwner);
	pStream->Read(&m_routeToCity.iID);

	pStream->Read(NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	pStream->Read(NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	pStream->Read(&m_bForceEmphasizeCulture);
	pStream->Read(NUM_CITY_PLOTS, m_aiBestBuildValue);
	pStream->Read(NUM_CITY_PLOTS, (int*)m_aeBestBuild);
	pStream->Read(GC.getNumEmphasizeInfos(), m_pbEmphasize);
	pStream->Read(NUM_YIELD_TYPES, m_aiSpecialYieldMultiplier);
	pStream->Read(&m_iCachePlayerClosenessTurn);
	pStream->Read(&m_iCachePlayerClosenessDistance);
	pStream->Read(MAX_PLAYERS, m_aiPlayerCloseness);
	pStream->Read(&m_iNeededFloatingDefenders);
	pStream->Read(&m_iNeededFloatingDefendersCacheTurn);
	pStream->Read(&m_iWorkersNeeded);
	pStream->Read(&m_iWorkersHave);
}

//
//
//
void CvCityAI::write(FDataStreamBase* pStream)
{
	CvCity::write(pStream);

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iEmphasizeAvoidGrowthCount);
	pStream->Write(m_iEmphasizeGreatPeopleCount);
	pStream->Write(m_bAssignWorkDirty);
	pStream->Write(m_bChooseProductionDirty);

	pStream->Write(m_routeToCity.eOwner);
	pStream->Write(m_routeToCity.iID);

	pStream->Write(NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	pStream->Write(NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	pStream->Write(m_bForceEmphasizeCulture);
	pStream->Write(NUM_CITY_PLOTS, m_aiBestBuildValue);
	pStream->Write(NUM_CITY_PLOTS, (int*)m_aeBestBuild);
	pStream->Write(GC.getNumEmphasizeInfos(), m_pbEmphasize);
	pStream->Write(NUM_YIELD_TYPES, m_aiSpecialYieldMultiplier);
	pStream->Write(m_iCachePlayerClosenessTurn);
	pStream->Write(m_iCachePlayerClosenessDistance);
	pStream->Write(MAX_PLAYERS, m_aiPlayerCloseness);
	pStream->Write(m_iNeededFloatingDefenders);
	pStream->Write(m_iNeededFloatingDefendersCacheTurn);
	pStream->Write(m_iWorkersNeeded);
	pStream->Write(m_iWorkersHave);
}
