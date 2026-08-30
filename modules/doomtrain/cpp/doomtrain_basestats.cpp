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
        const int32 baseStatMin =
            settings::get<int32>("doomtrain.BASE_STAT_MIN");

        const int32 baseStatMax =
            settings::get<int32>("doomtrain.BASE_STAT_MAX");
        
        const int32 maxStatPurchases =
            baseStatMax - baseStatMin;

        const int32 maximumPurchases =
            maxStatPurchases * 7;
        
        const double maxSPPerStat =
        (
            static_cast<double>(baseStatMin + 1) +
            static_cast<double>(baseStatMax)
        )
        *
        static_cast<double>(maxStatPurchases)
        / 2.0;
        
        const uint16 dt_str = PChar->getCharVar("doomtrain_str");
        const uint16 dt_dex = PChar->getCharVar("doomtrain_dex");
        const uint16 dt_vit = PChar->getCharVar("doomtrain_vit");
        const uint16 dt_agi = PChar->getCharVar("doomtrain_agi");
        const uint16 dt_int = PChar->getCharVar("doomtrain_int");
        const uint16 dt_mnd = PChar->getCharVar("doomtrain_mnd");
        const uint16 dt_chr = PChar->getCharVar("doomtrain_chr");

        const int32 strPurchases =
            std::clamp<int32>(dt_str, 0, maxStatPurchases);

        const int32 dexPurchases =
            std::clamp<int32>(dt_dex, 0, maxStatPurchases);

        const int32 vitPurchases =
            std::clamp<int32>(dt_vit, 0, maxStatPurchases);

        const int32 agiPurchases =
            std::clamp<int32>(dt_agi, 0, maxStatPurchases);

        const int32 intPurchases =
            std::clamp<int32>(dt_int, 0, maxStatPurchases);

        const int32 mndPurchases =
            std::clamp<int32>(dt_mnd, 0, maxStatPurchases);

        const int32 chrPurchases =
            std::clamp<int32>(dt_chr, 0, maxStatPurchases);

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

        const int32 totalStatPurchases =
            strPurchases +
            dexPurchases +
            vitPurchases +
            agiPurchases +
            intPurchases +
            mndPurchases +
            chrPurchases;

        //--------------------------------------------------
        // Total SP spent on base stats
        //--------------------------------------------------

        const auto calculateStatSPSpent =
            [baseStatMin](int32 purchases) -> double
        {
            if (purchases <= 0)
            {
                return 0.0;
            }

            const double firstCost =
                static_cast<double>(baseStatMin + 1);

            const double lastCost =
                static_cast<double>(baseStatMin + purchases);

            return (firstCost + lastCost)
                * static_cast<double>(purchases)
                / 2.0;
        };

        const double strSpent =
            calculateStatSPSpent(strPurchases);

        const double dexSpent =
            calculateStatSPSpent(dexPurchases);

        const double vitSpent =
            calculateStatSPSpent(vitPurchases);

        const double agiSpent =
            calculateStatSPSpent(agiPurchases);

        const double intSpent =
            calculateStatSPSpent(intPurchases);

        const double mndSpent =
            calculateStatSPSpent(mndPurchases);

        const double chrSpent =
            calculateStatSPSpent(chrPurchases);

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

        const double startingHP =
            settings::get<float>("doomtrain.STARTING_HP");

        const double maximumHP =
            settings::get<float>("doomtrain.MAX_HP");

        const double strHPWeight =
            settings::get<float>("doomtrain.STR_HP_WEIGHT");

        const double dexHPWeight =
            settings::get<float>("doomtrain.DEX_HP_WEIGHT");

        const double vitHPWeight =
            settings::get<float>("doomtrain.VIT_HP_WEIGHT");

        const double agiHPWeight =
            settings::get<float>("doomtrain.AGI_HP_WEIGHT");

        const double intHPWeight =
            settings::get<float>("doomtrain.INT_HP_WEIGHT");

        const double mndHPWeight =
            settings::get<float>("doomtrain.MND_HP_WEIGHT");

        const double chrHPWeight =
            settings::get<float>("doomtrain.CHR_HP_WEIGHT");

        // Every purchase receives at least 1 HP.
        const double guaranteedHP =
            static_cast<double>(totalStatPurchases);

        // HP remaining after starting HP and guaranteed
        // +1-per-purchase growth.
        const double weightedHPRange =
            maximumHP -
            startingHP -
            maximumPurchases;

        const double currentWeightedSP =
            (strSpent * strHPWeight) +
            (dexSpent * dexHPWeight) +
            (vitSpent * vitHPWeight) +
            (agiSpent * agiHPWeight) +
            (intSpent * intHPWeight) +
            (mndSpent * mndHPWeight) +
            (chrSpent * chrHPWeight);

        // Maximum possible weighted SP investment.
        const double maxWeightedSP =
            (maxSPPerStat * strHPWeight) +
            (maxSPPerStat * dexHPWeight) +
            (maxSPPerStat * vitHPWeight) +
            (maxSPPerStat * agiHPWeight) +
            (maxSPPerStat * intHPWeight) +
            (maxSPPerStat * mndHPWeight) +
            (maxSPPerStat * chrHPWeight);

        const double weightedHP =
            (currentWeightedSP / maxWeightedSP) *
            weightedHPRange;

        PChar->health.maxhp = static_cast<uint32>(
            std::floor(
                startingHP +
                guaranteedHP +
                weightedHP
            )
        );
        
        // =========================================================
        // MP CALCULATION
        // =========================================================

        const double startingMP =
            settings::get<float>("doomtrain.STARTING_MP");

        const double maximumMP =
            settings::get<float>("doomtrain.MAX_MP");

        const double strMPWeight =
            settings::get<float>("doomtrain.STR_MP_WEIGHT");

        const double dexMPWeight =
            settings::get<float>("doomtrain.DEX_MP_WEIGHT");

        const double vitMPWeight =
            settings::get<float>("doomtrain.VIT_MP_WEIGHT");

        const double agiMPWeight =
            settings::get<float>("doomtrain.AGI_MP_WEIGHT");

        const double intMPWeight =
            settings::get<float>("doomtrain.INT_MP_WEIGHT");

        const double mndMPWeight =
            settings::get<float>("doomtrain.MND_MP_WEIGHT");

        const double chrMPWeight =
            settings::get<float>("doomtrain.CHR_MP_WEIGHT");


        // ---------------------------------------------------------
        // Maximum Possible Weighted MP Investment
        // ---------------------------------------------------------
        //
        // Each stat can receive 254 purchases:
        //
        // displayed stat: 1 -> 255
        // char var:       0 -> 254
        //
        // Only stats with a non-zero MP weight contribute.

        const double maxStatPurchasesDouble =
            static_cast<double>(maxStatPurchases);

        const double maxWeightedMP =
            (maxStatPurchasesDouble * strMPWeight) +
            (maxStatPurchasesDouble * dexMPWeight) +
            (maxStatPurchasesDouble * vitMPWeight) +
            (maxStatPurchasesDouble * agiMPWeight) +
            (maxStatPurchasesDouble * intMPWeight) +
            (maxStatPurchasesDouble * mndMPWeight) +
            (maxStatPurchasesDouble * chrMPWeight);


        // ---------------------------------------------------------
        // Current Weighted MP Investment
        // ---------------------------------------------------------

        const double currentWeightedMP =
            (strPurchases * strMPWeight) +
            (dexPurchases * dexMPWeight) +
            (vitPurchases * vitMPWeight) +
            (agiPurchases * agiMPWeight) +
            (intPurchases * intMPWeight) +
            (mndPurchases * mndMPWeight) +
            (chrPurchases * chrMPWeight);


        // ---------------------------------------------------------
        // Scale Weighted Investment Into MP Range
        // ---------------------------------------------------------

        double maxMP = startingMP;

        if (maxWeightedMP > 0.0)
        {
            const double mpProgress =
                currentWeightedMP / maxWeightedMP;

            maxMP +=
                mpProgress * (maximumMP - startingMP);
        }


        // ---------------------------------------------------------
        // Apply Final MP
        // ---------------------------------------------------------

        PChar->health.maxmp = static_cast<int32>(
            std::clamp(
                std::floor(maxMP),
                startingMP,
                maximumMP
            )
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