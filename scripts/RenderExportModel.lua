models_str = RunCommand('getModels', {})
models = SplitString(models_str['msg'], ',')

sel_model = PromptSelection(models,'Select Model')

RunCommand('renderAll', {})

properties = {}
properties['model'] = sel_model
properties['format'] = 'avicompressed'
properties['filename'] = sel_model .. ".mp4"
Log(properties['filename'])

result = RunCommand('exportModel', properties)
Log(result['msg'])

