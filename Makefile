UNAME := $(shell uname)
ifeq ($(findstring CYGWIN,$(UNAME)), CYGWIN)
    ARCH := Cygwin
else
    ARCH := $(UNAME)
endif

#DEBUG = 1
#DEBUG_LEAKS = 1
#PROFILE = 1
#STATISTICS = 1

#OMP = 1
#CUDA  = 1
#OPENCL= 1
#FFTW3 = 1
EXR   = 1
GIF   = 1
JPEG  = 1
PNG   = 1
#TIFF  = 1

#FTGL  = 1
#OPENAL = 1
#BOX_2D = 1
#CHIPMUNK = 1
#PARDISO = 1
#OPENCV = 1

#DEFINES += -DRANGE_QUERY
#DEFINES += -DRAY_DIFFERENTIALS

DEFINES += -DGAUSSIAN_FILTER
#DEFINES += -DSTOP_RADIUS

#DEFINES = -DDISPERSION
#DEFINES = -DDUMP_PERFORMANCE
#DEFINES = -DGLOSSY_PHOTONS
#DEFINES = -DGLOSSY_HACK
#DEFINES = -DSIGGRAPH_KERNEL


CFLAGS += -Wno-error=deprecated-declarations

ifndef CUDA
CPPFLAGS += -MMD
endif
CPPFLAGS += $(DEFINES)

ifdef PROFILE
    CFLAGS   += -pg
    LDFLAGS  += -pg
    CXXFLAGS += -pg
else
ifdef DEBUG
    CFLAGS += -g -DDEBUG
    CFLAGS += -rdynamic # backtrace: no effect?
#    LDLIBS += -lefence
else
    CFLAGS += -O3 -DNDEBUG
ifeq ($(ARCH), Darwin)
    ifndef CUDA
    LDFLAGS += -dead_strip
    endif
endif
endif
endif

ifdef DEBUG_LEAKS
    CFLAGS += -DDEBUG_LEAKS
endif

CFLAGS += -std=c99# -pedantic -Werror -Wall -fno-strict-aliasing
#CFLAGS += -std=c99 -pedantic -Werror -Wall -Wextra -fno-strict-aliasing -Wno-unused-parameter -fno-strict-aliasing

ifdef STATISTICS
    CFLAGS += -DSTATISTICS
endif

ifeq ($(ARCH), Linux)
    CFLAGS   += -Wno-unused-result
    CPPFLAGS += -DLINUX
    CPPFLAGS += -D_BSD_SOURCE
    CPPFLAGS += -D_POSIX_SOURCE -D_POSIX_C_SOURCE
    CPPFLAGS += -I/usr/local/include -I/usr/local/include/freetype2

    EXR_FLAGS += -I/usr/include/OpenEXR
    GIF_LIBS = -lungif

    OPENGL_LIBS = -lglut -lGL -lGLU
    OPENAL_LIBS = -lalut -lopenal
    NV_OPENGL_LIBS = $(OPENGL_LIBS)
    NV_OPENAL_LIBS = $(OPENAL_LIBS)
    CUDA_INCLUDES = -I/usr/local/cuda/include
    CUDA_LIBS = -L/usr/local/cuda/lib64 -lcudart

    LDFLAGS += -z muldefs
    LDLIBS += -lstdc++ -L$(HOME)/usr/lib
endif

ifeq ($(ARCH), Darwin)
    CPPFLAGS += -DDARWIN
# old Mac
#    CPPFLAGS += -I/usr/X11/include -I/usr/X11/include/freetype2
#    CPPFLAGS += -I/usr/local/include/freetype2
    CPPFLAGS += -I/opt/local/include/freetype2
    CPPFLAGS += -I/opt/local/include
    EXR_FLAGS += -I/opt/local/include/OpenEXR
#    EXR_FLAGS += -I/usr/local/include/OpenEXR
#    ARCHFLAGS = -m64

#    GIF_LIBS = -lungif # old mac
    GIF_LIBS = -lgif

    NV_OPENGL_LIBS = -Xlinker -framework,GLUT -Xlinker -framework,OpenGL
    NV_OPENAL_LIBS = -Xlinker -framework,OpenAL

    OPENGL_LIBS = -framework GLUT -framework OpenGL
    OPENAL_LIBS = -framework OpenAL
    OPENCL_LIBS = -framework OpenCL

    CUDA_INCLUDES = -I/usr/local/cuda/include
    CUDA_LIBS = -L/usr/local/cuda/lib -lcudart -lcuda

#    LDLIBS += -L/usr/X11/lib -L/opt/local/lib -lstdc++.6
#    LDLIBS += -stdlib=libc++ -L/usr/X11/lib -L/usr/local/lib 
#    LDLIBS += -std=c++11 -stdlib=libc++ -L/usr/X11/lib -L/opt/local/lib
#    LDLIBS += -stdlib=libstdc++ -L/usr/X11/lib -L/opt/local/lib
#    LDLIBS += -L/usr/X11/lib -L/opt/local/lib -llibc++
    LDLIBS += -L/usr/X11/lib -L/opt/local/lib

     #LD = clang++
#     CXX = clang++
#     LINK.c = clang++
#     LINK.cc = clang++
#     LINK.cpp = clang++
#     LINK.C = clang++
     LINK.o = clang++
endif

ifeq ($(ARCH), Cygwin)
    CPPFLAGS += -DCYGWIN
#    CPPFLAGS += -D_BSD_SOURCE
#    CPPFLAGS += -D_POSIX_SOURCE -D_POSIX_C_SOURCE
    CPPFLAGS += -I/usr/include/freetype2

    GIF_LIBS = -lgif

    OPENGL_LIBS = -lglut -lglu32 -lGL
    #OPENGL_LIBS = -lglut32 -lglu32 -lopengl32
endif

CFLAGS    += $(ARCHFLAGS)
CXXFLAGS  += $(ARCHFLAGS)
LDFLAGS   += $(ARCHFLAGS)

NVCCFLAGS += $(ARCHFLAGS)
NVLDFLAGS += $(ARCHFLAGS)

ifdef OMP
    CPPFLAGS += -DOMP
    CFLAGS   += -fopenmp
    LDFLAGS  += -fopenmp
endif

ifdef CUDA
    NVCC      = nvcc
    CPPFLAGS += -DCUDA
    CPPFLAGS += $(CUDA_INCLUDES)
    LDLIBS   += $(CUDA_LIBS)
    CUDA_SOURCES = $(wildcard *.cu)
   #OBJECTS += $(notdir $(CUDA_SOURCES:.cu=.o))
    OBJECTS += $(CUDA_SOURCES:.cu=.o)
endif

ifdef OPENAL
    CPPFLAGS += -DOPENAL
#    LDLIBS   += $(OPENAL_LIBS)
endif

ifdef FFTW3
    CPPFLAGS += -DFFTW3 # -std=c99
    LDLIBS   += -lfftw3
#    LDLIBS   += -lfftw3f
endif

ifdef EXR
    CPPFLAGS += -DEXR
    CPPFLAGS += $(EXR_FLAGS)
    LDLIBS   += -lIlmImf -lIex
endif

ifdef GIF
    CPPFLAGS += -DGIF
    LDLIBS   += $(GIF_LIBS)
endif

ifdef JPEG
    CPPFLAGS += -DJPEG
    LDLIBS   += -ljpeg
endif

ifdef PNG
    CPPFLAGS += -DPNG
    LDLIBS   += -lpng
endif

ifdef FTGL
    CPPFLAGS += -DFTGL
    LDLIBS   += -lftgl
endif

ifdef TIFF
    CPPFLAGS += -DTIFF
    LDLIBS   += -ltiff
endif

ifdef PARDISO
    CPPFLAGS += -DPARDISO
    LDLIBS   += -L$(HOME)/usr/lib -lpardiso
ifeq ($(ARCH), Linux)
    LDLIBS   += -llapack -lgfortran
endif
endif

ifdef BOX_2D
    CPPFLAGS += -DBOX_2D
    LDLIBS   += -lBox2D
endif

ifdef CHIPMUNK
    CPPFLAGS += -DCHIPMUNK -I/opt/local/include/chipmunk
    LDLIBS   += -lchipmunk
endif

ifdef OPENCV
    CPPFLAGS += -DOPENCV
    LDLIBS   += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect
endif

CPPFLAGS  += -DVERTEX_BUFFER_OBJECT
LDLIBS    += -lfreetype -lm

NV_LDLIBS  := $(NV_OPENGL_LIBS) $(NV_OPENAL_LIBS) $(LDLIBS)
LDLIBS     :=    $(OPENGL_LIBS)    $(OPENAL_LIBS) $(OPENCL_LIBS) $(LDLIBS)

SOURCES     = $(wildcard *.c) gen_ext.c
CPP_SOURCES = $(wildcard *.cc)

OBJECTS += $(SOURCES:.c=.o)
OBJECTS += $(CPP_SOURCES:.cc=.o)

LIBRARY = libcuboid.a
TARGETS = iv
#CUDA_TARGETS = convolve_cuda nlmeans_cuda nlmeans_cuda_2 wavelet_threshold reconstruct

.SUFFIXES: .cu

default: $(TARGETS)
	$(RM) render.d

$(TARGETS): $(LIBRARY)

%: %.o $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^
ifeq ($(ARCH), Darwin)
	ranlib $(LIBRARY)
endif

clean:
	$(RM) $(TARGETS) $(LIBRARY) $(OBJECTS) *.d

.cu.o:
	$(NVCC) $(NVCCFLAGS) $(CPPFLAGS) -o $@ -c $<

$(CUDA_TARGETS):
	$(NVCC) $(NVLDFLAGS) $@.o $(LIBRARY) $(NV_LDLIBS) --host-compilation C++ -o $@

gen_ext.h: extensions.txt glext.h gen_h.sh
	./gen_h.sh $< > $@

gen_ext.c: extensions.txt glext.h gen_c.sh
	./gen_c.sh $< > $@

glext.h:
	wget http://www.opengl.org/registry/api/glext.h -O $@

render.o: render.glsl

-include $(notdir $(OBJECTS:.o=.d))

