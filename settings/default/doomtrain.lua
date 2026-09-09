-----------------------------------
-- DOOMTRAIN SERVER SETTINGS
-----------------------------------
-- Custom settings for the Doomtrain Server.
-----------------------------------

xi          = xi or {}
xi.settings = xi.settings or {}

xi.settings.doomtrain =
{
    -----------------------------------
    -- Progression
    -----------------------------------

    MAIN_JOB_CAP = 250,
    SUB_JOB_CAP  = 250,
    SP_GROWTH = 1,
    AP_GROWTH = 10,
    
    -----------------------------------
    -- Base Stats
    -----------------------------------
    
    BASE_STAT_MIN = 1,
    BASE_STAT_MAX = 255,
    
    STARTING_HP = 40,
    MAX_HP      = 9999,
    STR_HP_WEIGHT  = 5.0,
    DEX_HP_WEIGHT  = 6.0,
    VIT_HP_WEIGHT  = 10.0,
    AGI_HP_WEIGHT  = 8.0,
    INT_HP_WEIGHT  = 3.0,
    MND_HP_WEIGHT  = 5.0,
    CHR_HP_WEIGHT  = 3.0,
    
    STARTING_MP = 4,
    MAX_MP      = 999,
    STR_MP_WEIGHT  = 0.0,
    DEX_MP_WEIGHT  = 0.0,
    VIT_MP_WEIGHT  = 0.0,
    AGI_MP_WEIGHT  = 0.0,
    INT_MP_WEIGHT  = 10.0,
    MND_MP_WEIGHT  = 8.0,
    CHR_MP_WEIGHT  = 2.0,
    
    -----------------------------------
    -- Experience Points
    -----------------------------------
    
    EVEN_MATCH_BASE_EXP    = 50,
    STAT_EXP_MODIFIER    = 5,
    EXP_UNUSED_SP_MODIFIER  = 5,
    MIN_EXP        = 0,
}
