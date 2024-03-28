--
-- 
-- Select View from a sequence and Make it the Master View for selected Sequences
--

ShowMessage("First step ... Select a template sequence.")
template_seq = PromptSequences()
if #template_seq < 1 then
	Log("You must select a template sequence. Abort")
	return
end
if #template_seq > 1 then
	Log("You should only select 1 template sequence. Abort")
	return
end

properties = {}
properties['seq'] = template_seq[1]
Log("Template Sequence: "..template_seq[1])
properties['promptIssues'] = 'false'
result = RunCommand('openSequence', properties)

properties = {}
views_str = RunCommand('getViews', properties)

if views_str['res'] == 503 then
	Log(views_str['msg'])
    return -- Exit the script or the Lua interpreter
end

views = views_str['views']
sel_view = PromptSelection(views,'Select View')
Log("Status: "..sel_view)

if sel_view == "Master View" then
	Log("Already the master view, no work to do. Abort.")
	return
end

seqs = PromptSequences()

-- Close the template sequence
properties = {}
properties['quiet'] = 'true'
properties['force'] = 'true'	
result = RunCommand('closeSequence', properties)
Log("Status: "..result['msg'])
	
 for i,seq in ipairs(seqs) do 
	Log("Processing .. "..seq)
	--
	-- Open Sequence
	--
    properties = {}
    properties['seq'] = seq
	properties['promptIssues'] = 'false'
    result = RunCommand('openSequence', properties)
	Log("SequenceName: "..result['seq'].."Sequence Opened.")
	--
	-- Make the selected view, the master view
	--
    properties = {}
    properties['view'] = sel_view
    result = RunCommand('makeMaster', properties)
	Log("Status: "..result['msg'].." Code: "..result['res'].." View: "..sel_view)
	
	--
	-- Save the sequence
	--
	if result['res'] == 200 then
		properties = {}
		properties['seq'] = seq
		result = RunCommand('saveSequence', properties)
		Log("Status: "..result['msg'])
	else
		Log("Error: Sequence not saved.")
	end
	--
	-- Close the sequence
	--
	properties = {}
	properties['quiet'] = 'true'
	properties['force'] = 'true'	
	result = RunCommand('closeSequence', properties)
    Log("Status: "..result['msg'])
 end
Log("Done.")