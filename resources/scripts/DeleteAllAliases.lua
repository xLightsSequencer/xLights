properties = {}

deleteem = PromptOption('This will delete all aliases. Are you sure you want to proceed?','Delete aliases','Yes','No')

if deleteem == "Yes" then
	reallysure = PromptOption('Are you really sure you want to do this?', 'Confirm?', 'Delete', 'Cancel')
	if reallysure == "Delete" then
		result = RunCommand('deleteAllAliases', properties)
		if result['res'] == 200 then
			msg = ShowMessage('All aliases have been deleted!')
			Log("Following Groups/Models/SubModels had aliases that were removed:")
			Log(result['models'])
		else
			msg = ShowMessage('No aliases found.')
			Log(result['msg'])
		end
	else
		msg = ShowMessage('Smart move!')
	end
else
	msg = ShowMessage('Smart move!')
end
