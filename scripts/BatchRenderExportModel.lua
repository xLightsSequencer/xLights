models_str = RunCommand('getModels', {})
models = models_str['models']

sel_model = PromptSelection(models,'Select Model')
Log(sel_model)

seqs = PromptSequences()
for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq
	properties['promptIssues'] = 'false'
    result = RunCommand('openSequence', properties)

    properties = {}
	properties['highdef'] = 'true'	
	RunCommand('renderAll', properties)

    properties = {}
	properties['model'] = sel_model
	properties['format'] = 'avicompressed'
	
	media = string.gsub(result['media'],".*\\","")
	properties['filename'] = string.gsub(media,"%.mp%d","") .. ".mp4"
	Log(properties['filename'])

	result = RunCommand('exportModel', properties)
	Log(result['msg'])
	
	properties = {}
	properties['quiet'] = 'true'
	properties['force'] = 'true'	
    result = RunCommand('closeSequence', {})
    Log(result['msg'])
end




