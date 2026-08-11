-- BatchConvertVideoMedia.lua
-- 
-- Prompts for one or more sequences, then for each one transcodes any legacy
-- video (e.g. .avi) referenced by Video effects to a format the current
-- version of xLights supports, repoints the Video effects at the new files,
-- and saves the sequence so the conversion sticks.
-- Offers a practice run that only reports which sequences have media needing
-- conversion, without changing or saving any files.
--
-- PLEASE RUN A FILE->BACKUP PRIOR TO THIS SCRIPT


local proceed = PromptOption(
    'This script converts video media and re-saves the affected sequences.\n\n'
        .. 'PLEASE RUN A FILE->BACKUP PRIOR TO THIS SCRIPT.',
    'Backup Reminder', 'Continue', 'Cancel') == 'Continue'
if not proceed then return end

local seqs = PromptSequences()
if #seqs == 0 then ShowMessage('No sequences selected') return end

local dryRun = PromptOption(
    'Practice run only reports which sequences have media that needs converting, without changing any files.\n\n'
        .. 'Convert now will transcode the media and save the affected sequences.',
    'Practice Run or Convert?', 'Practice Run', 'Convert Now') == 'Practice Run'

local convertedCount = 0
local skippedCount = 0
local failCount = 0
local openFailCount = 0
local needsConversionCount = 0

-- Force-close whatever may be open, quietly. Used both before the loop (in
-- case a previous run/crash left a sequence open) and after every iteration,
-- so one bad sequence can never leave state behind that blocks the rest of
-- the batch from opening.
local function forceCloseAnyOpenSequence()
    local result = RunCommand('closeSequence', {quiet = 'true', force = 'true'})
    if result == nil then
        Log('  WARNING: closeSequence returned no result')
    end
    return result
end

forceCloseAnyOpenSequence()

for i, seq in ipairs(seqs) do
    Log('--- [' .. i .. '/' .. #seqs .. '] ' .. seq)

    local ok, err = pcall(function()
        local openResult = RunCommand('openSequence', {seq = seq, promptIssues = 'false'})
        if openResult == nil or openResult['seq'] == nil then
            Log('  ERROR: could not open sequence: ' .. ((openResult and openResult['msg']) or 'unknown error'))
            openFailCount = openFailCount + 1
            return
        end
        Log('  openSequence: ' .. openResult['seq'])

        local issues = RunCommand('getMediaIssues', {})
        local issueCount = 0
        if issues ~= nil and issues['issues'] ~= nil then
            issueCount = #issues['issues']
        end

        if issueCount == 0 then
            Log('  No media issues found, nothing to convert')
            skippedCount = skippedCount + 1
        elseif dryRun then
            needsConversionCount = needsConversionCount + 1
            Log('  [PRACTICE RUN] ' .. issueCount .. ' media issue(s) found, would attempt conversion:')
            for _, issue in ipairs(issues['issues']) do
                local convertible = issue['canconvert'] and 'convertible' or 'NOT convertible'
                Log('    - ' .. (issue['path'] or '?') .. ' (' .. (issue['type'] or '?') .. ', '
                    .. (issue['reason'] or '') .. ', ' .. convertible .. ')')
            end
        else
            local convertResult = RunCommand('convertMedia', {})
            if convertResult ~= nil and convertResult['res'] == 200 then
                Log('  convertMedia: ' .. (convertResult['msg'] or ''))
                convertedCount = convertedCount + 1

                local saveResult = RunCommand('saveSequence', {})
                if saveResult == nil or saveResult['res'] ~= 200 then
                    Log('  ERROR: saveSequence failed: ' .. ((saveResult and saveResult['msg']) or 'unknown error'))
                    failCount = failCount + 1
                else
                    Log('  saveSequence: ' .. (saveResult['msg'] or ''))
                end
            else
                Log('  ERROR: convertMedia failed: ' .. ((convertResult and convertResult['msg']) or 'unknown error'))
                failCount = failCount + 1
            end
        end
    end)

    if not ok then
        Log('  ERROR: unexpected script error while processing this sequence: ' .. tostring(err))
        failCount = failCount + 1
    end

    -- Always force-close, whether this iteration succeeded, failed, or threw,
    -- so the next openSequence never fails because this one is still open.
    local closeResult = forceCloseAnyOpenSequence()
    Log('  closeSequence: ' .. ((closeResult and closeResult['msg']) or 'unknown result'))
end

if dryRun then
    ShowMessage('Practice run complete: ' .. needsConversionCount .. ' sequence(s) need conversion, '
        .. skippedCount .. ' skipped (no issues), ' .. openFailCount .. ' could not be opened.\n\n'
        .. 'See the log for details. No files were changed.')
else
    ShowMessage('Batch video conversion complete: ' .. convertedCount .. ' converted, '
        .. skippedCount .. ' skipped (no issues), ' .. failCount .. ' failed, '
        .. openFailCount .. ' could not be opened.')
end
