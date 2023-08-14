-- Script to Export Video Preview, for the selected sequences.

seqs = PromptSequences()
for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq

    result = RunCommand('openSequence', properties)
    Log(result['seq']) 
    RunCommand('renderAll', {})
    result = RunCommand('exportVideoPreview', {})
    Log(result['msg'])
    Log(result['output'])
    result = RunCommand('closeSequence', {})
    Log(result['msg'])
end
