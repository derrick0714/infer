################################################################################
# You probably don't need to touch anything below this                         #
################################################################################

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

depend:
ifeq ($(CURDIR),$(SRC_DIR))
	for i in `find . -name '*.cpp'`; do \
		g++ -E -MT `echo $$i | sed -e 's/cpp$$/o/'` -MM $$i >> \
													$(OBJ_DIR)/makefile; \
	done;
else
	rm -f makefile
	$(MAKE) -C $(SRC_DIR) -f $(SRC_DIR)/Makefile.include \
		SRC_DIR=$(SRC_DIR) \
		OBJ_DIR=$(CURDIR) depend
endif

clean:
	rm -rf *

%.o : %.cpp
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) $(SYS_INCLUDE_DIRS) $(INCLUDE_DIRS) -o $@ -c $<

%: %.o
	$(CPP) -o $@ $^ $(SYS_LIB_DIRS) $(STATIC_LIBS) $(DYNAMIC_LIBS)

-include makefile
