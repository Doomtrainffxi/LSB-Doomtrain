-----------------------------------
-- func: doomtrain
-- desc: Doomtrain progression commands.
-----------------------------------

require('modules/module_utils')

---@type TCommand
local commandObj = {}

commandObj.cmdprops =
{
    permission = 0,
    parameters = 'ss',
}

local statVars =
{
    str = 'doomtrain_str',
    dex = 'doomtrain_dex',
    vit = 'doomtrain_vit',
    agi = 'doomtrain_agi',
    int = 'doomtrain_int',
    mnd = 'doomtrain_mnd',
    chr = 'doomtrain_chr',
}

local function printHelp(player)
    player:printToPlayer('!doomtrain stat <str|dex|vit|agi|int|mnd|chr>')
    player:printToPlayer('!doomtrain status')
end

commandObj.onTrigger = function(player, command, stat)

    -----------------------------------
    -- Status request
    -----------------------------------

    if command ~= nil and string.lower(command) == 'status' then
        local currentSP = player:getCharVar('doomtrain_sp_current')

        player:printToPlayer(
            string.format('DT_STATUS|SP|%d', currentSP)
        )

        return
    end
    
    -----------------------------------
    -- Validate command
    -----------------------------------

    -----------------------------------
    -- Validate command
    -----------------------------------

    if command == nil then
        printHelp(player)
        return
    end

    command = string.lower(command)

    -----------------------------------
    -- Points
    -----------------------------------

    if command == 'points' then

        local currentSP = player:getCharVar('doomtrain_sp_current')

        player:printToPlayer(
            string.format(
                'DOOMTRAIN_SP:%d',
                currentSP
            )
        )

        return
    end

    -----------------------------------
    -- Stat command validation
    -----------------------------------

    if command ~= 'stat' then
        printHelp(player)
        return
    end

    if stat == nil then
        printHelp(player)
        return
    end

    stat = string.lower(stat)

    local statVar = statVars[stat]

    if statVar == nil then
        player:printToPlayer('Invalid stat.')
        printHelp(player)
        return
    end

-----------------------------------
-- Get current values
-----------------------------------

local currentSP = player:getCharVar('doomtrain_sp_current')
local currentStat = player:getCharVar(statVar)

-----------------------------------
-- Validate stat cap
-----------------------------------

-- CharVar 254 + starting stat 1 = displayed stat 255.
if currentStat >= 254 then
    player:printToPlayer(
        string.format(
            '%s is already at the maximum of 255.',
            string.upper(stat)
        )
    )
    return
end

-----------------------------------
-- Calculate upgrade cost
-----------------------------------

-- The new displayed stat value is:
-- current CharVar + 1 for the current base stat
-- + 1 for this upgrade.
local upgradeCost = currentStat + 2

-----------------------------------
-- Validate Stat Points
-----------------------------------

if currentSP < upgradeCost then
    player:printToPlayer(
        string.format(
            'You need %d Stat Points to increase %s.',
            upgradeCost,
            string.upper(stat)
        )
    )
    return
end

-----------------------------------
-- Spend SP and increase stat
-----------------------------------

player:setCharVar(
    'doomtrain_sp_current',
    currentSP - upgradeCost
)

player:setCharVar(
    statVar,
    currentStat + 1
)

-- Immediately update the player's visible stats.
player:recalculateStats()

player:printToPlayer(
    string.format(
        'Spent %d Stat Points. %s is now %d.',
        upgradeCost,
        string.upper(stat),
        currentStat + 2
    )
)
end

-----------------------------------
-- Register command
-----------------------------------

xi.module.registerCommand('doomtrain', commandObj)