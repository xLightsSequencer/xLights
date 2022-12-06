### Lua Scripting API
The Script Runner Dialog allows the user to run [Lua](http://www.lua.org/manual/5.3/manual.html) Scripts to control xLights functions.

#### Lua Types
  - boolean true
  - string 'Sample'
  - number 1
  - table {a="Test", b=2}
  - function a=Print()

#### xLights Specific

##### Variables
  - "showfolder", string, Current Show Folder
  - "userscripsfolder", string, Scripts Folder in Show Folder
  - "systemscripsfolder", string, Scripts Folder in xLights Install Directory

##### Functions

| Name             | Input Parameters Types           | Return Types | Description                                 |
| ---------------- | -------------------------------- | ------------ | ------------------------------------------- |
| RunCommand       | string command, table parameters | table        | Calls xlDo Automation Command               |
| PromptSequences  |                                  | table        | Opens GUI to Select Sequence Files          |
| ShowMessage      | string message                   |              | Opens MessageBox with message               |
| PromptString     | string message                   | table        | Opens Text Entry Dialog for User Entry      |
| PromptSelection  | table item, string message       | string       | Opens Item Selection Dialog                 |
| SplitString      | string text, string delimiter    | table        | Splits single string into table of strings  |
| JoinString       | table items, string delimiter    | string       | Joins table of strings into single string   |
| JSONToTable      | string json                      | table        | Create Table of JSON Parameters Recursively |
| TableToJSON      | table items                      | string       | Create JSON of Table Parameters Recursively |
| Log              | string/number/boolean value      |              | Log Message to Script Dialog                |
