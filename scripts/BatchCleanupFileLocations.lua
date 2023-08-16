-- Script to Cleanup File Locations, for the selected sequences.

seqs = PromptSequences()
for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq

    result = RunCommand('openSequence', properties)
    Log(result['seq']) 
    result = RunCommand('cleanupFileLocations', {})
    Log(result['msg'])
    result = RunCommand('saveSequence', {})
    Log(result['msg'])
    result = RunCommand('closeSequence', {})
    Log(result['msg'])
end
result = RunCommand('saveLayout', {})
Log(result['msg']) 

