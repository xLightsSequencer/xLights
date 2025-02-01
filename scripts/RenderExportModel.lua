--
-- 
-- Select Model and Sequences to have them rendered and exported to mp4 files
-- This uses the force highdef rendering if set
--
local FILE_DELIMITER  = "\\"

properties = {}
properties['groups'] = "false"

models_str = RunCommand('getModels', properties)
models = models_str['models']
sel_model = PromptSelection(models,'Select Model')

sel_compress = PromptSelection({'Yes','No'},'Compressed MP4?')

seqs,sel_highdef = PromptSequences()

-- Function to check if a folder exists
function folderExists(path)
	Log("Checking folderExists(): "..path)
	if type(path)~="string" then return false end
	res = os.rename(path,path)
	return res
end

for i,seq in ipairs(seqs) do 
    properties = {}
    properties['seq'] = seq
	properties['promptIssues'] = 'false'
    result = RunCommand('openSequence', properties)
	Log("SequenceName: "..result['seq'])

	fullSequenceName = result['fullseq']
	Log("FullSeqName: "..fullSequenceName)
	
	lastSeparatorIndex = fullSequenceName:find("[\\/]([^\\/]*)$")
	sequenceRenderingLocation = fullSequenceName:sub(1,lastSeparatorIndex -1)
	Log("sequenceRenderingLocation: "..sequenceRenderingLocation)
		
	media = string.gsub(result['media'],".*\\","")
	if media == "" then 
		media = "export"
	end
	mediaFile = string.gsub(media,"%.mp%d","") .. ".mp4"
	Log("MediaFile: "..mediaFile)

	newFileName = sequenceRenderingLocation..FILE_DELIMITER..sel_model..FILE_DELIMITER..mediaFile
	--folder location of export video
	local folderPath = sequenceRenderingLocation..FILE_DELIMITER..sel_model
	
	-- Check if the folder exists
	if not folderExists(folderPath) then
		-- Folder doesn't exist, so create it
		local success, errorMessage = os.execute("mkdir " .. folderPath)

		if success then
			Log("The folder '"..sel_model.."' has been created at: " .. folderPath)
		else
			Log("Failed to create the folder '"..sel_model.."': " .. errorMessage)
		end
	else
		Log("The folder '"..sel_model.."' already exists at: " .. folderPath)
	end
			
    properties = {}
	properties['model'] = sel_model
	if sel_compress == "Yes" then
		Log("Compressed")
		properties['format'] = 'avicompressed'
	else
		Log("Uncompressed")
		properties['format'] = 'mp4uncompressed'
	end
	properties['filename'] = newFileName
	properties['highdef'] = tostring(sel_highdef)
	
	Log ("Export Model: "..sel_model)
	Log ("Export File: "..newFileName)
	Log ("Force HighDef: "..tostring(sel_highdef))
	
	result = RunCommand('exportModelWithRender', properties)
	Log("Export Result: "..result['msg'])
	
	properties = {}
	properties['quiet'] = 'true'
	properties['force'] = 'true'	
    result = RunCommand('closeSequence', {})
    Log("Status: "..result['msg'])
end
