#include "map/utils/moduleutils.h"
#include "map/utils/charutils.h"

#include "common/logging.h"

#include "map/entities/char_entity.h"
#include "map/entities/mob_entity.h"

#include <algorithm>
#include <cmath>

namespace
{
    /************************************************************************
     *                                                                       *
     *  Doomtrain Custom Stats                                               *
     *                                                                       *
     ************************************************************************/

    auto GetStatSPValue(uint16 stat) -> uint32
    {
        // A base stat of 1 requires 0 SP.
        //
        // 1 -> 2 costs 2 SP
        // 2 -> 3 costs 3 SP
        // ...
        //
        // Total SP required to reach a stat value:
        // (stat * (stat + 1) / 2) - 1

        if (stat <= 1)
        {
            return 0;
        }

        return (stat * (stat + 1) / 2) - 1;
    }

    auto CalculateCustomStats(CCharEntity* PChar) -> bool
    {
        const uint16 dt_str = PChar->getCharVar("doomtrain_str");
        const uint16 dt_dex = PChar->getCharVar("doomtrain_dex");
        const uint16 dt_vit = PChar->getCharVar("doomtrain_vit");
        const uint16 dt_agi = PChar->getCharVar("doomtrain_agi");
        const uint16 dt_int = PChar->getCharVar("doomtrain_int");
        const uint16 dt_mnd = PChar->getCharVar("doomtrain_mnd");
        const uint16 dt_chr = PChar->getCharVar("doomtrain_chr");

        //--------------------------------------------------
        // Base stats
        //--------------------------------------------------

        PChar->stats.STR = 1 + dt_str;
        PChar->stats.DEX = 1 + dt_dex;
        PChar->stats.VIT = 1 + dt_vit;
        PChar->stats.AGI = 1 + dt_agi;
        PChar->stats.INT = 1 + dt_int;
        PChar->stats.MND = 1 + dt_mnd;
        PChar->stats.CHR = 1 + dt_chr;

        //--------------------------------------------------
        // Number of stat purchases
        //
        // Char vars begin at 0 and each purchase adds 1.
        // Therefore, the sum of the Doomtrain stat vars
        // is exactly the number of base-stat purchases.
        //--------------------------------------------------

        const uint32 totalStatPurchases =
            dt_str +
            dt_dex +
            dt_vit +
            dt_agi +
            dt_int +
            dt_mnd +
            dt_chr;

        //--------------------------------------------------
        // Total SP spent on base stats
        //--------------------------------------------------

        const uint32 strSP = GetStatSPValue(PChar->stats.STR);
        const uint32 dexSP = GetStatSPValue(PChar->stats.DEX);
        const uint32 vitSP = GetStatSPValue(PChar->stats.VIT);
        const uint32 agiSP = GetStatSPValue(PChar->stats.AGI);
        const uint32 intSP = GetStatSPValue(PChar->stats.INT);
        const uint32 mndSP = GetStatSPValue(PChar->stats.MND);
        const uint32 chrSP = GetStatSPValue(PChar->stats.CHR);

        //--------------------------------------------------
        // HP
        //
        // Base HP: 40
        //
        // HP growth is based on SP invested into each stat,
        // while preserving different stat weights.
        //--------------------------------------------------

        const double weightedSP =
        (strSP * 5.0) +
        (dexSP * 6.0) +
        (vitSP * 10.0) +
        (agiSP * 8.0) +
        (intSP * 3.0) +
        (mndSP * 5.0) +
        (chrSP * 3.0);

        //--------------------------------------------------
        // HP Scaling
        //
        // Start: 40 HP
        //
        // Every stat purchase guarantees +1 HP.
        //
        // Remaining HP is distributed according to the
        // weighted SP investment.
        //
        // Maximum stats = 9,999 HP.
        //--------------------------------------------------

        constexpr double startingHP = 40.0;
        constexpr double maximumHP  = 9999.0;

        // 7 stats × 254 purchases each.
        constexpr double maximumPurchases = 1778.0;

        // Every purchase receives at least 1 HP.
        const double guaranteedHP =
            static_cast<double>(totalStatPurchases);

        // HP remaining after starting HP and guaranteed
        // +1-per-purchase growth.
        constexpr double weightedHPRange =
            maximumHP -
            startingHP -
            maximumPurchases;

        // Maximum possible weighted SP investment.
        constexpr double maxWeightedSP =
            (32639.0 * 5.0) +
            (32639.0 * 6.0) +
            (32639.0 * 9.212) +
            (32639.0 * 8.0) +
            (32639.0 * 3.0) +
            (32639.0 * 5.0) +
            (32639.0 * 3.0);

        const double weightedHP =
            (weightedSP / maxWeightedSP) *
            weightedHPRange;

        PChar->health.maxhp = static_cast<uint32>(
            std::floor(
                startingHP +
                guaranteedHP +
                weightedHP
            )
        );
        
        //--------------------------------------------------
        // MP
        //--------------------------------------------------

        PChar->health.maxmp = std::ceil(
            ((1 + dt_int) * 1.5) +
            ((1 + dt_mnd) * 1.915) +
            ((1 + dt_chr) * 0.5)
        );
                
        return true;
    }

    /************************************************************************
     *                                                                       *
     *  Doomtrain Level Up                                                   *
     *                                                                       *
     ************************************************************************/

    void OnDoomtrainLevelUp(CCharEntity* PChar)
    {
        uint32 dt_sp_total   = PChar->getCharVar("doomtrain_sp_total");
        uint32 dt_sp_current = PChar->getCharVar("doomtrain_sp_current");
        uint32 dt_ap_total   = PChar->getCharVar("doomtrain_ap_total");
        uint32 dt_ap_current = PChar->getCharVar("doomtrain_ap_current");

        PChar->setCharVar("doomtrain_sp_total", dt_sp_total + 1);
        PChar->setCharVar("doomtrain_sp_current", dt_sp_current + 1);
        PChar->setCharVar("doomtrain_ap_total", dt_ap_total + 1);
        PChar->setCharVar("doomtrain_ap_current", dt_ap_current + 1);

        // Each Doomtrain level increases the subjob.
        // Once the subjob reaches 250, it resets and the
        // main job progresses through the existing level-up flow.
        if (PChar->jobs.job[static_cast<uint8>(PChar->GetSJob())] < 250)
        {
            PChar->jobs.job[static_cast<uint8>(PChar->GetSJob())] += 1;
            PChar->jobs.job[static_cast<uint8>(PChar->GetMJob())] += -1;
        }
        else
        {
            PChar->jobs.job[static_cast<uint8>(PChar->GetSJob())] = 1;
        }

        charutils::SaveCharJob(PChar, PChar->GetSJob());
    }

    /************************************************************************
     *                                                                       *
     *  Doomtrain Experience                                                 *
     *                                                                       *
     ************************************************************************/

    void CalculateDoomtrainExperience(
        CCharEntity* PChar,
        CMobEntity* PMob,
        uint32& baseExp)
    {
        //--------------------------------------------------
        // Player Total SP
        //--------------------------------------------------

        const uint32 playerTotalSP =
            PChar->getCharVar("doomtrain_sp_total");

        //--------------------------------------------------
        // Monster Total SP
        //--------------------------------------------------

        uint32 mobTotalSP = 0;

        mobTotalSP += GetStatSPValue(PMob->stats.STR);
        mobTotalSP += GetStatSPValue(PMob->stats.DEX);
        mobTotalSP += GetStatSPValue(PMob->stats.VIT);
        mobTotalSP += GetStatSPValue(PMob->stats.AGI);
        mobTotalSP += GetStatSPValue(PMob->stats.INT);
        mobTotalSP += GetStatSPValue(PMob->stats.MND);
        mobTotalSP += GetStatSPValue(PMob->stats.CHR);

        //--------------------------------------------------
        // Doomtrain EXP Formula
        //
        // EXP = 10 + Mob Total SP - Player Total SP
        //--------------------------------------------------

        int64 calculatedExp =
            10 +
            static_cast<int64>(mobTotalSP) -
            static_cast<int64>(playerTotalSP);

        // Never allow zero or negative EXP.
        calculatedExp = std::max<int64>(1, calculatedExp);

        baseExp = static_cast<uint32>(calculatedExp);

        //--------------------------------------------------
        // Temporary balancing/debug output
        //--------------------------------------------------

        ShowInfoFmt(
            "Doomtrain EXP | Player SP: {} | Mob SP: {} | EXP: {}",
            playerTotalSP,
            mobTotalSP,
            baseExp
        );
    }
}

/************************************************************************
 *                                                                       *
 *  Doomtrain Base Stats Module                                           *
 *                                                                       *
 ************************************************************************/

class DoomtrainBaseStatsModule : public CPPModule
{
public:
    void OnInit() override
    {
        ShowInfoFmt("Doomtrain Base Stats C++ module initialized");

        // Custom base stats, HP, and MP.
        moduleutils::RegisterCalculateStatsHandler(
            CalculateCustomStats
        );

        // Doomtrain progression and SP/AP gains.
        moduleutils::RegisterLevelUpHandler(
            OnDoomtrainLevelUp
        );

        // Doomtrain custom EXP calculation.
        moduleutils::RegisterCalculateDoomtrainExperienceHandler(
            CalculateDoomtrainExperience
        );
    }
};

REGISTER_CPP_MODULE(DoomtrainBaseStatsModule);