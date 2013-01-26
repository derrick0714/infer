SRC_DIR= $(CURDIR)
OBJ_DIR= ../build
DIST_DIR= ../dist
INSTALL_PREFIX= /usr/local

.SUFFIXES: 

all:

%:
	[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(MAKE) -C $(OBJ_DIR) -f $(SRC_DIR)/Makefile.include \
							SRC_DIR=$(SRC_DIR) \
							DIST_DIR=$(DIST_DIR) \
							INSTALL_PREFIX=$(INSTALL_PREFIX) \
							$@
