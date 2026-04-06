
ISPC_ARCH=$(shell uname -m)

ifeq '$(ISPC_ARCH)' 'aarch64'
ISPC_FLAGS=--target-os=linux --target=neon-i32x8 --arch=aarch64 --pic
else
ISPC_FLAGS=--target-os=linux --target=avx2-i32x16 --target=avx1-i32x16  --target=sse4.2-i32x8 --target=sse2-i32x8 --arch=x86_64 --pic
endif

$(OBJDIR_LINUX_DEBUG)/__/src-core/%.o: ../src-core/%.ispc
	@mkdir -p $(dir $@)
	../ispc -g $(ISPC_FLAGS) -o $@ $<

$(OBJDIR_LINUX_RELEASE)/__/src-core/%.o: ../src-core/%.ispc
	@mkdir -p $(dir $@)
	../ispc --math-lib=fast $(ISPC_FLAGS) -o $@ $<

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/LayerBlendingFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/LayerBlendingFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/ButterflyFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/ButterflyFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/PinwheelFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/PinwheelFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/PlasmaFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/PlasmaFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/VideoFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/VideoFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/BarsFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/BarsFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/ColorWashFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/ColorWashFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/FanFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/FanFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/KaleidoscopeFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/KaleidoscopeFunctions.o
