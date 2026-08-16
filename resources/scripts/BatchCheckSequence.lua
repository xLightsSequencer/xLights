-- Script to Check Sequence, for the selected sequences.
-- Opens a browser tab with the Check Sequence report for each sequence.

seqs = PromptSequences()
for i,seq in ipairs(seqs) do
    properties = {}
    properties['seq'] = seq
    properties['promptIssues'] = 'false'
    result = RunCommand('checkSequence', properties)
    Log(result['msg'])
    Log(result['output'])
    os.execute('start "" "' .. result['output'] .. '"')
end
