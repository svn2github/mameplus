# tiny compile
COREDEFS += -DTINY_NAME="driver_samsho2"
COREDEFS += -DTINY_POINTER="&driver_samsho2"

# CPUs
CPUS+=Z80@
CPUS+=M68000@
CPUS+=M68020@

# SOUNDs
SOUNDS+=AY8910@
SOUNDS+=YM2610@
SOUNDS+=YM2610B@

OBJS = \
	$(OBJ)/machine/neogeo.o $(OBJ)/machine/neocrypt.o $(OBJ)/machine/pd4990a.o $(OBJ)/vidhrdw/neogeo.o $(OBJ)/drivers/neogeo.o \

# MAME specific core objs
COREOBJS += $(OBJ)/tiny.o $(OBJ)/cheat.o
