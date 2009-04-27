###########################################################################
#
#   windows.mak
#
#   MESS Windows-specific makefile
#
###########################################################################



CFLAGS += -DEMULATORDLL=\"$(EMULATORDLL)\"
RCFLAGS += -DMESS

LIBS += -lcomdlg32

OBJDIRS += \
	$(MESSOBJ)/osd \
	$(MESSOBJ)/osd/windows

MESS_WINSRC = $(SRC)/mess/osd/windows
MESS_WINOBJ = $(OBJ)/mess/osd/windows

LIBOSD += \
	$(MESS_WINOBJ)/configms.o	\
	$(MESS_WINOBJ)/dialog.o	\
	$(MESS_WINOBJ)/menu.o		\
	$(MESS_WINOBJ)/opcntrl.o

$(LIBOSD): $(OSDOBJS)

OSDCOREOBJS += \
	$(OBJ)/mess/osd/windows/winmess.o	\
	$(OBJ)/mess/osd/windows/winutils.o	\
	$(OBJ)/mess/osd/windows/glob.o

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOCORE_NOMAIN): $(OSDCOREOBJS:$(WINOBJ)/main.o=)



#-------------------------------------------------
# rules for resource file
#-------------------------------------------------

ifeq ($(NO_DLL),)
    $(MESS_WINOBJ)/messlib.res: $(MESS_WINSRC)/mess.rc $(WINOBJ)/mamevers.rc
    LIBOSD += $(MESS_WINOBJ)/messlib.res
else
    ifeq ($(WINUI),)
        $(MESS_WINOBJ)/messcli.res: $(MESS_WINSRC)/mess.rc $(WINSRC)/mame.rc $(WINOBJ)/mamevers.rc
        CLIRESFILE = $(MESS_WINOBJ)/messcli.res
    endif
endif



#-------------------------------------------------
# generic rules for the resource compiler
#-------------------------------------------------

$(MESS_WINOBJ)/%.res: $(MESS_WINSRC)/%.rc
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) --include-dir mess/$(OSD) -o $@ -i $<


