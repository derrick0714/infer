SRC_DIR= $(CURDIR)
OBJ_DIR= ../obj

.SUFFIXES: 

all:

%:
	[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(MAKE) -C $(OBJ_DIR) -f $(SRC_DIR)/Makefile.include SRC_DIR=$(SRC_DIR) $(MAKECMDGOALS)
