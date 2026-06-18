-- BatchImportSequences.lua
-- Imports sequences (.xsq) or packages (.xsqz/.zip/.piz) from any source folder
-- into a destination show, using Auto Map or a mapping file.
--
-- The source folder does NOT need to be an xLights show.  Any folder containing
-- .xsq, .xsqz, .zip, or .piz files is accepted.

local sep = package.config:sub(1,1)  -- '/' on Mac/Linux, '\' on Windows

local sourceFolder = PromptString('Source folder containing sequences or packages')
if sourceFolder == '' then ShowMessage('Cancelled') return end

local destFolder = PromptString('Destination show folder path')
if destFolder == '' then ShowMessage('Cancelled') return end

local listResult = RunCommand('listSequences', {folder=sourceFolder})
if listResult == nil or listResult['sequences'] == nil or #listResult['sequences'] == 0 then
    ShowMessage('No sequences or packages found in:\n' .. sourceFolder)
    return
end

-- Build display names and a lookup map for selection
local allSeqs = listResult['sequences']
local displayNames = {}
local nameToSeq = {}
for _, seq in ipairs(allSeqs) do
    local label = seq['name'] .. '  [' .. seq['type'] .. ']  ' .. seq['path']
    table.insert(displayNames, label)
    nameToSeq[label] = seq
end

-- Let the user pick which files to import (all pre-selected)
local chosen = PromptMultiSelection(displayNames, 'Select sequences to import (' .. #displayNames .. ' found)')
if #chosen == 0 then ShowMessage('No files selected') return end

-- Resolve back to seq objects in original order
local seqs = {}
for _, label in ipairs(chosen) do
    table.insert(seqs, nameToSeq[label])
end
Log('Selected ' .. #seqs .. ' of ' .. #allSeqs .. ' file(s)')

local mapMethod = PromptSelection({'Auto Map', 'Load Map', 'Both'}, 'Model mapping method?')
if mapMethod == '' then ShowMessage('Cancelled') return end
local mapFile = ''
if mapMethod == 'Load Map' or mapMethod == 'Both' then
    mapFile = PromptString('Full path to .xmap or .xjmap mapping file')
    if mapFile == '' then ShowMessage('Cancelled') return end
end

local successCount = 0
local failCount = 0
for i, seq in ipairs(seqs) do
    Log('--- [' .. i .. '/' .. #seqs .. '] ' .. seq['path'])

    local info = RunCommand('getSequenceInfo', {filename=seq['path']})
    if info == nil or info['name'] == nil then
        Log('ERROR: Could not read sequence info')
        failCount = failCount + 1
    else
        Log('  name=' .. info['name'] .. '  duration=' .. info['duration'] .. 'ms  frameRate=' .. info['frameRate'] .. 'fps')

        local result = RunCommand('changeShowFolder', {folder=destFolder, force='true'})
        Log('  changeShowFolder: ' .. (result['msg'] or ''))

        -- newSequence expects durationSecs (seconds) and frameMS (ms per frame)
        local durationSecs = math.floor(info['duration'] / 1000)
        local frameMS      = math.floor(1000 / info['frameRate'])
        result = RunCommand('newSequence', {
            mediaFile    = info['mediaFile'],
            durationSecs = tostring(durationSecs),
            frameMS      = tostring(frameMS)
        })
        Log('  newSequence: ' .. (result['msg'] or ''))

        local mmValue = 'file'
        if mapMethod == 'Auto Map' then mmValue = 'auto'
        elseif mapMethod == 'Both' then mmValue = 'both'
        end
        local importParams = {
            filename    = seq['path'],
            importmedia = 'true',
            mapmethod   = mmValue
        }
        if mapFile ~= '' then importParams['mapfile'] = mapFile end
        result = RunCommand('importXLightsSequence', importParams)
        Log('  importXLightsSequence: ' .. (result['msg'] or ''))

        -- saveSequence needs the full path for a new (unnamed) sequence
        local savePath = destFolder .. sep .. info['name'] .. '.xsq'
        result = RunCommand('saveSequence', {seq=savePath})
        Log('  saveSequence: ' .. (result['msg'] or '') .. ' -> ' .. savePath)

        result = RunCommand('closeSequence', {quiet='true', force='true'})
        Log('  closeSequence: ' .. (result['msg'] or ''))

        successCount = successCount + 1
    end
end

ShowMessage('Batch import complete: ' .. successCount .. ' succeeded, ' .. failCount .. ' failed.')
