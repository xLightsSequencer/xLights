
ISPC_ARCH=$(shell uname -m)

ifeq '$(ISPC_ARCH)' 'aarch64'
ISPC_FLAGS=--target-os=linux --target=neon-i32x8 --arch=aarch64 --pic
else
ISPC_FLAGS=--target-os=linux --target=sse2-i32x8 --target=avx2-i32x8 --target=avx2-i32x16 --target=avx512icl-x32 --arch=x86_64 --pic
endif

$(OBJDIR_LINUX_DEBUG)/%.o: %.ispc
	../ispc -g $(ISPC_FLAGS) -o $@ $<

$(OBJDIR_LINUX_RELEASE)/%.o: %.ispc
	../ispc --math-lib=fast $(ISPC_FLAGS) -o $@ $<

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/effects/ispc/LayerBlendingFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/effects/ispc/LayerBlendingFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/effects/ispc/ButterflyFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/effects/ispc/ButterflyFunctions.o
