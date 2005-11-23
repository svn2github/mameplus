# a tiny compile is without Neogeo games
COREDEFS += -DTINY_COMPILE=1
COREDEFS += -DTINY_NAME="driver_grdians"
COREDEFS += -DTINY_POINTER="&driver_grdians"

# uses these CPUs
CPUS+=M68000@
CPUS+=M68020@

# uses these SOUNDs
SOUNDS+=YMZ280B@
SOUNDS+=X1_010@

OBJS =  $(OBJ)/drivers/seta2.o $(OBJ)/vidhrdw/seta2.o

# MAME specific core objs
COREOBJS += $(OBJ)/driver.o $(OBJ)/cheat.o
