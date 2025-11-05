-- Script to open, render and close the selected sequences. 

seqs = PromptSequences()
local start_time = os.clock() -- Start timing
for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq
    properties['promptIssues'] = 'false'
    properties['force'] = 'true'
    result = RunCommand('openSequence', properties)
    Log('Render ' .. result['seq']) 

    local start_time_seq = os.clock() -- Start timing
    properties = {}
    properties['highdef'] = 'false'
    result = RunCommand('renderAll', properties)
    Log('Message: ' .. result['msg'])
    local end_time = os.clock() -- End timing	
    Log(string.format("Render Elapsed time: %.4f seconds for %s", end_time - start_time_seq, seq))

    properties = {}
    properties['quiet'] = 'true'
    properties['force'] = 'true'
    result = RunCommand('closeSequence', properties)
    Log(result['msg'])
end
local end_time = os.clock() -- End timing	
Log(string.format("Render Elapsed time: %.4f seconds", end_time - start_time))