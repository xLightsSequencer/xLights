-- TestHeadlessRenderOptions.lua
-- Windows only. Renders the selected sequence(s) multiple times using the
-- --headless command line mode of a separate xLights.exe, toggling GPU
-- rendering, hardware video decoding, and (when HW video decode is on) the
-- HW video renderer backend (DirectX11 vs FFmpeg Auto), then reports timing
-- for each combination plus a recommended (fastest) setting. This is a
-- timing benchmark only - it does not verify render output is unchanged
-- between configurations.
--
-- The preview graphics backend (OpenGL vs Vulkan) is deliberately NOT tested
-- here: it only selects the interactive preview window's canvas backend
-- (xlVulkanCanvas::VulkanSelected) and has no code path in --headless at all,
-- so it can't affect a headless render.
--
-- Each headless run gets its own isolated APPDATA folder (via the APPDATA
-- env var) pre-seeded with a tiny settings.json, so this never reads or
-- writes the real xLights settings on this machine.

local function fileExists(path)
    local f = io.open(path, 'rb')
    if f then f:close() return true end
    return false
end

local function writeFile(path, data)
    local f = io.open(path, 'w')
    if not f then return false end
    f:write(data)
    f:close()
    return true
end

local function mkdirp(path)
    os.execute('mkdir "' .. path .. '" 2>NUL')
end

local function baseName(path)
    return path:match('([^\\/]+)$') or path
end

local function stripExt(name)
    return (name:gsub('%.[^.]+$', ''))
end

-- Ask Windows for the path of the xLights.exe that is already running (i.e.
-- the one executing this very script) and offer it as the default, rather
-- than making the user type/browse for a path every time.
local function detectRunningExePath()
    local ph = io.popen('powershell -NoProfile -NonInteractive -Command "(Get-Process -Name xLights -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Path)"', 'r')
    if not ph then return nil end
    local out = ph:read('*a')
    ph:close()
    if out == nil then return nil end
    out = out:gsub('^%s+', ''):gsub('%s+$', '')
    if out == '' then return nil end
    return out
end

local exePath = nil
local detected = detectRunningExePath()
if detected ~= nil and fileExists(detected) then
    local choice = PromptOption('Use the currently running xLights.exe as the build to test?\n\n' .. detected, 'xLights.exe to test', 'Use this', 'Choose another')
    if choice == 'Use this' then exePath = detected end
end
if exePath == nil then
    exePath = PromptString('Full path to the xLights.exe to test (e.g. the Release build under xLights\\x64\\Release):')
end
if exePath == nil or exePath == '' then ShowMessage('Cancelled') return end
if not fileExists(exePath) then ShowMessage('Could not find file: ' .. exePath) return end

local seqs = PromptSequences()
if #seqs == 0 then ShowMessage('No sequences selected') return end

local repeatStr = PromptSelection({'1', '2', '5'}, 'How many times should EACH configuration be run per sequence (averaged for timing)?')
if repeatStr == nil or repeatStr == '' then ShowMessage('Cancelled') return end
local repeatCount = tonumber(repeatStr) or 1

-- Read the real xLights preferences (%APPDATA%\xLights\settings.json,
-- section "main") so we can label whichever tested combination matches what
-- this machine is actually set to. Only READS that file - never written to,
-- so the user's real preferences are untouched.
local function readCurrentPreferences()
    local appData = os.getenv('APPDATA')
    if not appData then return nil end
    local f = io.open(appData .. '\\xLights\\settings.json', 'r')
    if not f then return nil end
    local data = f:read('*a')
    f:close()
    local ok, t = pcall(JSONToTable, data)
    if not ok or type(t) ~= 'table' or type(t['main']) ~= 'table' then return nil end
    local main = t['main']
    local function toBool(v, default)
        if v == nil then return default end
        if type(v) == 'boolean' then return v end
        if type(v) == 'number' then return v ~= 0 end
        if type(v) == 'string' then return v == 'true' or v == '1' or v == 'True' or v == 'yes' end
        return default
    end
    local backend = main['xLightsGraphicsBackend']
    if type(backend) ~= 'string' or backend == '' then backend = 'Auto' end
    local renderer = main['xLightsVideoReaderRenderer']
    local hwRenderer = 0
    if type(renderer) == 'number' then hwRenderer = math.floor(renderer)
    elseif type(renderer) == 'string' then hwRenderer = tonumber(renderer) or 0 end
    return {
        gpu = toBool(main['xLightsGPURendering'], true),
        hwVideo = toBool(main['xLightsVideoReaderAccelerated'], true),
        backend = backend,
        hwRenderer = hwRenderer,
    }
end

-- Matches HardwareVideoRenderChoice's item order in OtherSettingsPanel.cpp.
local HW_RENDERER_NAMES = { [0] = 'DirectX11', [1] = 'FFmpeg Auto', [2] = 'FFmpeg CUDA', [3] = 'FFmpeg QSV', [4] = 'FFmpeg Vulkan', [5] = 'FFmpeg AMF', [6] = 'FFmpeg DirectX11' }

local currentPrefs = readCurrentPreferences()
local currentBackend = (currentPrefs and currentPrefs.backend) or 'Auto'
local currentRenderer = (currentPrefs and currentPrefs.hwRenderer) or 0
if currentPrefs == nil then
    Log('Could not read current preferences from settings.json - backend/renderer written to each test config will default to Auto/DirectX11 (backend is irrelevant to --headless anyway).')
end

-- Full 2x2 factorial of absolute combinations, rather than toggles relative
-- to a single "baseline" - GPU rendering and hardware video decode are
-- independent settings, so "disable GPU" is a different, and equally
-- interesting, test whether the machine currently has HW video on or off.
-- Whichever combination matches this machine's real preferences is labeled
-- as such rather than run twice under two names. The HW video renderer
-- (DirectX11 vs FFmpeg) only matters when HW video decode is ON, so it's a
-- separate pair of configs rather than another factor in this grid.
local ALL_CONFIGS = {
    { key = 'gpu_on_hw_on',   gpu = true,  hwVideo = true },
    { key = 'gpu_on_hw_off',  gpu = true,  hwVideo = false },
    { key = 'gpu_off_hw_on',  gpu = false, hwVideo = true },
    { key = 'gpu_off_hw_off', gpu = false, hwVideo = false },
}
for _, c in ipairs(ALL_CONFIGS) do
    c.backend = currentBackend
    c.hwRenderer = currentRenderer
    c.label = string.format('GPU rendering %s, HW video decode %s', c.gpu and 'ON' or 'OFF', c.hwVideo and 'ON' or 'OFF')
    if currentPrefs ~= nil and currentPrefs.gpu == c.gpu and currentPrefs.hwVideo == c.hwVideo then
        c.label = c.label .. ' (your current preferences)'
        c.isCurrent = true
    end
end

-- HW video decode renderer: keep GPU rendering and backend at their current
-- preference values so this isolates just the decoder backend.
local RENDERERS_TO_TEST = { { key = 'hwvideo_dx11', index = 0 }, { key = 'hwvideo_ffauto', index = 1 } }
for _, r in ipairs(RENDERERS_TO_TEST) do
    local c = {
        key = r.key,
        gpu = (currentPrefs and currentPrefs.gpu) or true,
        hwVideo = true,
        hwRenderer = r.index,
        backend = currentBackend,
        label = 'HW video decode ON - ' .. HW_RENDERER_NAMES[r.index],
    }
    if currentPrefs ~= nil and currentPrefs.hwVideo and currentPrefs.hwRenderer == r.index then
        c.label = c.label .. ' (matches your current renderer)'
        c.isCurrentRenderer = true
    end
    table.insert(ALL_CONFIGS, c)
end

local labelList = {}
local labelToConfig = {}
for _, c in ipairs(ALL_CONFIGS) do
    table.insert(labelList, c.label)
    labelToConfig[c.label] = c
end

local chosenLabels = PromptMultiSelection(labelList, 'Select configurations to test (GPU rendering, HW video decode on/off, HW video renderer)')
if #chosenLabels == 0 then ShowMessage('No configurations selected') return end

local configs = {}
for _, label in ipairs(chosenLabels) do
    table.insert(configs, labelToConfig[label])
end

local testRoot = showfolder .. '\\HeadlessRenderTest'
local appDataRoot = testRoot .. '\\AppData'
local fseqRoot = testRoot .. '\\fseq'
local logRoot = testRoot .. '\\logs'
mkdirp(testRoot)
mkdirp(appDataRoot)
mkdirp(fseqRoot)
mkdirp(logRoot)

local function writeTempSettings(configKey, gpu, hwVideo, backend, hwRenderer)
    local dir = appDataRoot .. '\\' .. configKey .. '\\xLights'
    mkdirp(dir)
    local json = string.format(
        '{"main": {"xLightsGPURendering": %s, "xLightsVideoReaderAccelerated": %s, "xLightsGraphicsBackend": "%s", "xLightsVideoReaderRenderer": %d}}',
        tostring(gpu), tostring(hwVideo), backend, hwRenderer or 0)
    writeFile(dir .. '\\settings.json', json)
end

local function runHeadless(config, seqPath, runIdx)
    local seqBase = stripExt(baseName(seqPath))
    local tempAppData = appDataRoot .. '\\' .. config.key
    writeTempSettings(config.key, config.gpu, config.hwVideo, config.backend, config.hwRenderer)

    local runOutDir = fseqRoot .. '\\' .. seqBase .. '\\' .. config.key .. '\\run' .. runIdx
    mkdirp(runOutDir)
    local logPath = logRoot .. '\\' .. seqBase .. '_' .. config.key .. '_run' .. runIdx .. '.log'

    local cmd = 'set APPDATA=' .. tempAppData .. '&&"' .. exePath .. '" --headless -s "' .. showfolder .. '" --outputdir "' .. runOutDir .. '" "' .. seqPath .. '" > "' .. logPath .. '" 2>&1'

    local startTime = os.time()
    local ok = os.execute(cmd)
    local elapsed = os.difftime(os.time(), startTime)

    return {
        ok = (ok == true),
        elapsed = elapsed,
        log = logPath,
    }
end

local totalRuns = #seqs * #configs * repeatCount
Log(string.format('Running %d headless render(s): %d sequence(s) x %d configuration(s) x %d repeat(s)', totalRuns, #seqs, #configs, repeatCount))

-- results[seqPath][configKey] = { runs = { {elapsed=, ok=, log=}, ... } }
local results = {}
local runNum = 0
for _, seqPath in ipairs(seqs) do
    results[seqPath] = {}
    for _, config in ipairs(configs) do
        local runs = {}
        for runIdx = 1, repeatCount do
            runNum = runNum + 1
            Log(string.format('[%d/%d] %s - %s (run %d)', runNum, totalRuns, baseName(seqPath), config.label, runIdx))
            local r = runHeadless(config, seqPath, runIdx)
            Log(string.format('  -> %s in %.0f second(s)', r.ok and 'completed' or 'FAILED', r.elapsed))
            table.insert(runs, r)
        end
        results[seqPath][config.key] = runs
    end
end

-- Build the report and per-config aggregate totals used for the recommendation.
local report = {}
local function addLine(s) table.insert(report, s) Log(s) end

local totalElapsed = {}
for _, config in ipairs(configs) do
    totalElapsed[config.key] = 0
end

addLine('=================================================================')
addLine('Headless Render Option Test - ' .. os.date())
addLine('Executable: ' .. exePath)
addLine('=================================================================')

for _, seqPath in ipairs(seqs) do
    addLine('')
    addLine('Sequence: ' .. baseName(seqPath))
    for _, config in ipairs(configs) do
        local runs = results[seqPath][config.key]
        local sum = 0
        local allOk = true
        for _, r in ipairs(runs) do
            sum = sum + r.elapsed
            if not r.ok then allOk = false end
        end
        local avg = sum / #runs
        totalElapsed[config.key] = totalElapsed[config.key] + sum

        addLine(string.format('  %-11s avg %6.1fs over %d run(s), %s',
            config.key, avg, #runs, allOk and 'ok' or 'FAILED'))
    end
end

addLine('')
addLine('--- Recommendation ---')
addLine('Note: this is a timing comparison only - it does not check whether render output changed between configurations.')

local divisor = #seqs * repeatCount
local bestKey, bestAvg, currentKey, currentAvg = nil, nil, nil, nil
for _, config in ipairs(configs) do
    local avg = totalElapsed[config.key] / divisor
    if bestAvg == nil or avg < bestAvg then bestKey, bestAvg = config.key, avg end
    if config.isCurrent then currentKey, currentAvg = config.key, avg end
end

if currentKey ~= nil and bestKey == currentKey then
    addLine('Recommended: keep your current preferences - it was the fastest combination tested.')
elseif currentKey ~= nil then
    local pctFaster = currentAvg > 0 and ((currentAvg - bestAvg) / currentAvg * 100) or 0
    local bestLabel = bestKey
    for _, config in ipairs(configs) do
        if config.key == bestKey then bestLabel = config.label end
    end
    addLine(string.format("Recommended: %s (%.0f%% faster than your current preferences, %.1fs vs %.1fs avg). Spot-check the rendered fseq/preview before relying on this, since output was not compared.",
        bestLabel, pctFaster, bestAvg, currentAvg))
else
    local bestLabel = bestKey
    for _, config in ipairs(configs) do
        if config.key == bestKey then bestLabel = config.label end
    end
    addLine(string.format('Recommended: %s (%.1fs avg) - fastest combination tested. Your current preferences were not among the tested configurations, so no relative speedup could be computed.',
        bestLabel, bestAvg))
end

-- Called out separately from the overall fastest-combination recommendation
-- above, since "which HW video renderer" is its own actionable question even
-- when the renderer axis isn't the single fastest config overall.
local dxAvg = totalElapsed['hwvideo_dx11'] and (totalElapsed['hwvideo_dx11'] / divisor) or nil
local ffAvg = totalElapsed['hwvideo_ffauto'] and (totalElapsed['hwvideo_ffauto'] / divisor) or nil
if dxAvg ~= nil and ffAvg ~= nil then
    if dxAvg == ffAvg then
        addLine(string.format('HW video renderer: DirectX11 and FFmpeg Auto were the same speed (%.1fs avg each).', dxAvg))
    elseif dxAvg < ffAvg then
        local pct = ffAvg > 0 and ((ffAvg - dxAvg) / ffAvg * 100) or 0
        addLine(string.format('HW video renderer: use DirectX11 (%.1fs avg, %.0f%% faster than FFmpeg Auto at %.1fs avg).', dxAvg, pct, ffAvg))
    else
        local pct = dxAvg > 0 and ((dxAvg - ffAvg) / dxAvg * 100) or 0
        addLine(string.format('HW video renderer: use FFmpeg Auto (%.1fs avg, %.0f%% faster than DirectX11 at %.1fs avg).', ffAvg, pct, dxAvg))
    end
elseif dxAvg ~= nil or ffAvg ~= nil then
    addLine('HW video renderer: only one of DirectX11/FFmpeg Auto was tested - select both to compare them.')
end

local reportPath = testRoot .. '\\report.txt'
writeFile(reportPath, table.concat(report, '\n'))
Log('Full report written to: ' .. reportPath)

ShowMessage('Headless render option test complete.\n\nSee the script log and:\n' .. reportPath)
