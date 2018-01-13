
echo "#ifndef __TOOLBAR__IMAGES" > toolbarImages.h
echo "#define __TOOLBAR__IMAGES" >> toolbarImages.h

 ls -1 *.xpm | awk '//{print "#include \"../include/toolbar/" $0 "\""}' >> toolbarImages.h

echo "#endif" >> toolbarImages.h

