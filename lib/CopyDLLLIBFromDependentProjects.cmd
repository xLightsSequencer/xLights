rem copies files into the lib folders and bin folders based on standard build

rem assumes wxWidgets is installed in the ../wxWidgets folder relative to the root xlights folder

rem 64 bit

rem wxWidgets
copy ..\..\wxWidgets\lib\vc_x64_lib\wxbase31u.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxbase31ud.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxbase31u_net.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxbase31ud_net.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxbase31u_xml.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxbase31ud_xml.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxexpat.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxexpatd.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxjpeg.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxjpegd.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31u_aui.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31ud_aui.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31u_core.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31ud_core.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31u_gl.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31ud_gl.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31u_html.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31ud_html.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31u_media.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31ud_media.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31u_qa.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxmsw31ud_qa.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxpng.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxpngd.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxregexu.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxregexud.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxscintilla.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxscintillad.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxtiff.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxtiffd.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxzlib.lib .\windows64
copy ..\..\wxWidgets\lib\vc_x64_lib\wxzlibd.lib .\windows64

goto skip64_gcc_wx

copy ..\..\wxWidgets\lib\gcc_dll\libwxbase31u.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxbase31u_net.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxbase31u_xml.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxexpat.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxjpeg.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_aui.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_core.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_gl.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_html.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_propgrid.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_qa.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxpng.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxregexu.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxscintilla.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxtiff.a .\windows64
copy ..\..\wxWidgets\lib\gcc_dll\libwxzlib.a .\windows64

copy ..\..\wxWidgets\lib\gcc_dll\wxbase314u_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxbase314u_net_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxbase314u_xml_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_aui_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_core_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_gl_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_html_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_propgrid_gcc_custom.dll ..\bin64
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_qa_gcc_custom.dll ..\bin64

:skip64_gcc_wx

rem 32 bit

rem wxWidgets
copy ..\..\wxWidgets\lib\vc_lib\wxbase31u.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxbase31ud.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxbase31u_net.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxbase31ud_net.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxbase31u_xml.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxbase31ud_xml.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxexpat.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxexpatd.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxjpeg.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxjpegd.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31u_aui.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31ud_aui.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31u_core.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31ud_core.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31u_gl.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31ud_gl.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31u_html.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31ud_html.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31u_media.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31ud_media.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31u_qa.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxmsw31ud_qa.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxpng.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxpngd.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxregexu.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxregexud.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxscintilla.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxscintillad.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxtiff.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxtiffd.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxzlib.lib .\windows
copy ..\..\wxWidgets\lib\vc_lib\wxzlibd.lib .\windows

goto skip32_gcc_wx

copy ..\..\wxWidgets\lib\gcc_dll\libwxbase31u.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxbase31u_net.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxbase31u_xml.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxexpat.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxjpeg.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_aui.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_core.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_gl.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_html.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_propgrid.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxmsw31u_qa.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxpng.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxregexu.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxscintilla.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxtiff.a .\windows
copy ..\..\wxWidgets\lib\gcc_dll\libwxzlib.a .\windows

copy ..\..\wxWidgets\lib\gcc_dll\wxbase314u_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxbase314u_net_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxbase314u_xml_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_aui_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_core_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_gl_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_html_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_propgrid_gcc_custom.dll ..\bin
copy ..\..\wxWidgets\lib\gcc_dll\wxmsw314u_qa_gcc_custom.dll ..\bin

:skip32_gcc_wx

pause