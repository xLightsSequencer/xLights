-- Script to Export Model MP4 for Selected Model. 

properties2 = {}
properties2['groups'] = "false"

models_str = RunCommand('getModels', properties2)
models = models_str['models']

sel_model = PromptSelection(models,'Select Model')

RunCommand('renderAll', {})

properties = {}
properties['model'] = sel_model
properties['format'] = 'avicompressed'
properties['filename'] = sel_model .. ".mp4"
Log(properties['filename'])
Log(properties['model'])

result = RunCommand('exportModel', properties)
Log(result['msg'])

