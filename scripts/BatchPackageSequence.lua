-- Script to Package Sequence, for the selected sequences. 

seqs = PromptSequences()
for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq

    result = RunCommand('openSequence', properties)
    Log(result['seq']) 
    result = RunCommand('packageSequence', {})
    Log(result['msg'])
    Log(result['output'])
    result = RunCommand('closeSequence', {})
    Log(result['msg'])
end
