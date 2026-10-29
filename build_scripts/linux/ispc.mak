
ISPC_ARCH=$(shell uname -m)

ifeq '$(ISPC_ARCH)' 'aarch64'
ISPC_FLAGS=--target-os=linux --target=neon-i32x8 --arch=aarch64 --pic
else
# One comma-separated --target list: ispc keeps only the last of repeated
# --target flags. Multi-target writes a dispatcher object plus one object per
# ISA (_avx2/_avx/_sse4/_sse2); they are merged into the single $@ with ld -r.
ISPC_FLAGS=--target-os=linux --target=avx2-i32x8,avx1-i32x8,sse4.2-i32x8,sse2-i32x8 --arch=x86_64 --pic
endif

$(OBJDIR_LINUX_DEBUG)/__/src-core/%.o: ../src-core/%.ispc
	@mkdir -p $(dir $@) $(@:.o=.ispcobjs)
	@rm -f $(@:.o=.ispcobjs)/*.o
	../ispc -g $(ISPC_FLAGS) -o $(@:.o=.ispcobjs)/$(notdir $@) $<
	ld -r -o $@ $(@:.o=.ispcobjs)/*.o

$(OBJDIR_LINUX_RELEASE)/__/src-core/%.o: ../src-core/%.ispc
	@mkdir -p $(dir $@) $(@:.o=.ispcobjs)
	@rm -f $(@:.o=.ispcobjs)/*.o
	../ispc $(ISPC_FLAGS) -o $(@:.o=.ispcobjs)/$(notdir $@) $<
	ld -r -o $@ $(@:.o=.ispcobjs)/*.o

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

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/ShockwaveFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/ShockwaveFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/SpiralsFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/SpiralsFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/CirclesFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/CirclesFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/WarpFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/WarpFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/GalaxyFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/GalaxyFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/MeteorsFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/MeteorsFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/GarlandsFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/GarlandsFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/FillFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/FillFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/FireFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/FireFunctions.o

OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/TreeFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/TreeFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/TwinkleFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/TwinkleFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/ShimmerFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/ShimmerFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/CandleFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/CandleFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/LifeFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/LifeFunctions.o
OBJ_LINUX_DEBUG +=  $(OBJDIR_LINUX_DEBUG)/__/src-core/effects/ispc/WaveFunctions.o
OBJ_LINUX_RELEASE +=  $(OBJDIR_LINUX_RELEASE)/__/src-core/effects/ispc/WaveFunctions.o
