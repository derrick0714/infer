.SUFFIXES:

SYS_INCLUDE_DIRS:= $(SYS_INCLUDE_DIRS:%=-isystem %)
SYS_LIB_DIRS:=$(SYS_LIB_DIRS:%=-L%)
INCLUDE_DIRS:= $(INCLUDE_DIRS:%=-I%)

ifneq ($(strip $(DYNAMIC_LIBS)),)
DYNAMIC_LIBS:= -Wl,-Bdynamic $(DYNAMIC_LIBS:%=-l%)
endif
ifneq ($(strip $(STATIC_LIBS)),)
STATIC_LIBS:= -Wl,-Bstatic $(STATIC_LIBS:%=-l%)
endif

VPATH= $(SRC_DIR)

clean:
	rm -rf *

dist-clean:
	rm -rf $(DIST_DIR)/*

include/%.o : CFLAGS := $(CFLAGS) -fPIC
include/%.o : CXXFLAGS := $(CXXFLAGS) -fPIC
lib/%.o : CFLAGS := $(CFLAGS) -fPIC
lib/%.o : CXXFLAGS := $(CXXFLAGS) -fPIC
lib%.so : LDFLAGS := $(LDFLAGS) -shared -fPIC \
								-Xlinker -z -Xlinker now

%.o : %.c
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SYS_INCLUDE_DIRS) $(INCLUDE_DIRS) -o $@ -c $<

%.o : %.cpp
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SYS_INCLUDE_DIRS) $(INCLUDE_DIRS) -o $@ -c $<

lib%.so : %.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(SYS_LIB_DIRS) $(STATIC_LIBS) $(DYNAMIC_LIBS)

% : %.o
	$(CXX) -o $@ $^ $(SYS_LIB_DIRS) $(STATIC_LIBS) $(DYNAMIC_LIBS)

-include makefile
