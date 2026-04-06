-- Script to copy effects from one model to another model, for the selected sequences.
--
properties2 = {}
properties2['groups'] = "true"

models_str = RunCommand('getModels', properties2)
models = models_str['models']

sel_src_model = PromptSelection(models,'Select Source Model')
Log(sel_src_model)
sel_tgt_model = PromptSelection(models,'Select Target Model')
Log(sel_tgt_model)
seqs = PromptSequences()

for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq
	properties['promptIssues'] = 'false'
	properties['force'] = 'true'
    result = RunCommand('openSequence', properties) -- No return value
	Log(result)
	
    properties = {}
	properties['source'] = sel_src_model
    properties['target'] = sel_tgt_model
	properties['eraseModel'] = 'true'
	
    result = RunCommand('cloneModelEffects', properties)
	Log(result['msg'])
	
	result = RunCommand('saveSequence', {})
    Log(result['msg'])

    result = RunCommand('closeSequence', {})
    Log(result['msg'])
end