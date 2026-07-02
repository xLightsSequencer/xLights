-- Upload FPP Connect configuration to a selected FPP controller.
-- FPP must be discoverable on the network (same subnet).

-- Get all configured controllers
local ctrl_result = RunCommand('getControllers', {})

if ctrl_result['res'] ~= 200 then
    ShowMessage('Failed to get controllers: ' .. (ctrl_result['msg'] or 'unknown error'))
    return
end

-- Build a list of Ethernet controllers that have an IP address
local controllers = {}
local display_names = {}

if ctrl_result['controllers'] then
    for _, c in ipairs(ctrl_result['controllers']) do
        if c['type'] == 'Ethernet' and c['ip'] and c['ip'] ~= '' then
            table.insert(controllers, c)
            table.insert(display_names, c['name'] .. '  [' .. c['ip'] .. ']')
        end
    end
end

if #controllers == 0 then
    ShowMessage('No Ethernet controllers with IP addresses found in the current layout.')
    return
end

-- Let the user pick which controller to upload to
local selected_name = PromptSelection(display_names, 'Select FPP controller to upload to:')
if selected_name == '' then
    return
end

local selected_controller = nil
for i, name in ipairs(display_names) do
    if name == selected_name then
        selected_controller = controllers[i]
        break
    end
end

if selected_controller == nil then
    ShowMessage('Controller not found.')
    return
end

local ip = selected_controller['ip']
Log('Selected: ' .. selected_controller['name'] .. '  [' .. ip .. ']')

-- Ask what to upload
local udp_choice = PromptOption(
    'Upload UDP output configuration?',
    'UDP Outputs',
    'All outputs',
    'Skip'
)

local models_choice = PromptOption(
    'Upload model memory maps?',
    'Model Maps',
    'Yes',
    'No'
)

local map_choice = PromptOption(
    'Upload virtual display map?',
    'Display Map',
    'Yes',
    'No'
)

-- Map button labels to parameter values
local udp_val = ''
if udp_choice == 'All outputs' then
    udp_val = 'all'
end

local models_val = 'false'
if models_choice == 'Yes' then
    models_val = 'true'
end

local map_val = 'false'
if map_choice == 'Yes' then
    map_val = 'true'
end

if udp_val == '' and models_val == 'false' and map_val == 'false' then
    ShowMessage('Nothing selected to upload.')
    return
end

Log('Uploading FPP config to ' .. ip .. ' ...')

local params = {}
params['ip'] = ip
params['udp'] = udp_val
params['models'] = models_val
params['displayMap'] = map_val

local result = RunCommand('uploadFPPConfig', params)

if result['res'] == 200 then
    ShowMessage('Upload complete: ' .. (result['msg'] or 'Success.'))
else
    ShowMessage('Upload failed (' .. tostring(result['res']) .. '): ' .. (result['msg'] or 'Unknown error.'))
end
