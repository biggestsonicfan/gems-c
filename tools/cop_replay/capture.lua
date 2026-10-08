-- Capture the COP firmware's side of its FIFOs off MAME, from power-on to some
-- frames into attract's replay fight, for gems_cop_replay. The format is
-- m2-hle2's tools/mame/cop-capture.lua's (tests/cop_replay.c reads it):
--   <out>.bin         (u32 tag, u32 value) records in bus order:
--                       0x21000000 a command word (read at the command loop, PM 0x20141)
--                       0x20000000 an argument word the running command read
--                       0x30000000 a word the firmware wrote to its output FIFO
--                       0x900000..0x97FFFF the i960 wrote bufferram
--                       0x4000PPOO + 12 x 0x41000000  with CAP_SNAP=1: the current
--                         matrix before command OO, as command PP left it (value:
--                         its DM address), for gems_cop_replay's RESYNC
--   <out>.bufram.bin  bufferram, and <out>.dm.bin SHARC DM 0x30000-0x30FFF, at the start
-- written as it goes (no table of millions of records). From power-on, so the
-- replay builds the COP's state from the commands, as the board did.
--
-- It jumps attract straight to its Sonic vs Bean replay, as m2-hle2's
-- tools/mame/match-replay.lua does, then stops CAP_FIGHT frames later.
--
--   CAP_OUT=<prefix> CAP_FIGHT=600 [CAP_SNAP=1] mame sfight -rompath <zips> -nodrc -video none
--     -sound none -nothrottle -skip_gameinfo -seconds_to_run 299
--     -autoboot_script tools/cop_replay/capture.lua
--
-- -nodrc: the tap reads the SHARC's PC. -seconds_to_run under 300 skips MAME's
-- "this system doesn't work" notice.

local OUT   = assert(os.getenv("CAP_OUT"), "set CAP_OUT")
local FIGHT = tonumber(os.getenv("CAP_FIGHT") or "600")
local SNAP  = os.getenv("CAP_SNAP") == "1"

local sp  = manager.machine.devices[":maincpu"].spaces["program"]
local cop = manager.machine.devices[":copro_adsp"].spaces["data"]
local pcst = manager.machine.devices[":copro_adsp"].state["PC"]

local function dump(path, a0, a1, space)
    local f, c = assert(io.open(path, "wb")), {}
    for a = a0, a1, (space == sp and 4 or 1) do c[#c + 1] = string.pack("<I4", space:read_u32(a)) end
    f:write(table.concat(c))
    f:close()
end
dump(OUT .. ".bufram.bin", 0x900000, 0x91fffc, sp)
dump(OUT .. ".dm.bin", 0x30000, 0x30fff, cop)

local f = assert(io.open(OUT .. ".bin", "wb"))
local chunk, n = {}, 0
local function rec(tag, val)
    n = n + 1
    chunk[#chunk + 1] = string.pack("<I4I4", tag, val)
    if #chunk == 8192 then f:write(table.concat(chunk)); chunk = {} end
end

local prev = 0
local taps = {
    cop:install_read_tap(0x0400000, 0x0bfffff, "cap_in", function(offset, data, mask)
        if pcst.value ~= 0x20141 then rec(0x20000000, data) return data end
        if SNAP then
            local m = cop:read_u32(0x3033f)
            rec(0x40000000 | (prev << 8) | (data & 0xff), m)
            for k = 0, 11 do rec(0x41000000, cop:read_u32(m + k)) end
        end
        prev = data & 0xff
        rec(0x21000000, data)
        return data end),
    cop:install_write_tap(0x0c00000, 0x13fffff, "cap_out", function(offset, data, mask) rec(0x30000000, data) end),
    sp:install_write_tap(0x00900000, 0x0097ffff, "cap_bufw", function(offset, data, mask) rec(offset, data) end),
}

-- The jump to the replay fight (match-replay.lua), at the frame edge.
local STEP_ADDR, FROM_STEP, TO_STEP = 0x500030, 5, 6
local READY_ADDR, STATE_ADDR = 0x5004CC, 0x5004C4
local STATE = { 0x000301A7, 0x00000028, 0x00055DDC, 0x000562D0, 0xC1200000,
                0x433A8000, 0x43810000, 0xC1200000, 0x43398000 }
local jumped, frames, done = false, 0, false
local log = assert(io.open(OUT .. ".log", "w"))

local function finish()
    if done then return end
    done = true
    for _, t in ipairs(taps) do t:remove() end
    if #chunk > 0 then f:write(table.concat(chunk)) end
    f:close()
    log:write(string.format("done: %d records, %d fight frames\n", n, frames))
    log:close()
    manager.machine:exit()
end

local function edge(offset, data, mask)
    if done then return nil end
    if not jumped then
        if sp:read_u8(STEP_ADDR) == FROM_STEP and sp:read_u32(READY_ADDR) ~= 0 then
            for i, w in ipairs(STATE) do sp:write_u32(STATE_ADDR + 4 * (i - 1), w) end
            sp:write_u8(STEP_ADDR, TO_STEP)
            jumped = true
            log:write(string.format("jumped at frame_counter %d, record %d\n", sp:read_u32(0x500020), n))
            log:flush()
        end
        return nil
    end
    frames = frames + 1
    if frames % 120 == 0 then log:write(string.format("fight frame %d, %d records\n", frames, n)); log:flush() end
    if frames >= FIGHT then finish() end
    return nil
end
_G.CAP_EDGE = sp:install_write_tap(0x50D000, 0x50D003, "cap_edge", edge)
