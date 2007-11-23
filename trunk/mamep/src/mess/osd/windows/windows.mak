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

MESS_WINSRC = src/mess/osd/windows
MESS_WINOBJ = $(OBJ)/mess/osd/windows

#fixme: should use LIBOSD +=
MESSLIBOSD += \
	$(MESS_WINOBJ)/configms.o	\
	$(MESS_WINOBJ)/dialog.o	\
	$(MESS_WINOBJ)/menu.o		\
	$(MESS_WINOBJ)/opcntrl.o	\
	$(MESS_WINOBJ)/tapedlg.o

ifeq ($(NO_DLL),)
    $(MESS_WINOBJ)/messlib.res: $(MESS_WINSRC)/mess.rc $(WINOBJ)/mamevers.rc
    MESSLIBOSD += $(MESS_WINOBJ)/messlib.res
else
    ifneq ($(WINUI),)
        ## fixme: move gui resource code to mess/osd/ui
        RCFLAGS += --include-dir $(WINUISRC) --include-dir $(WINUIOBJ)
        $(MESS_WINOBJ)/messgui.res: $(MESS_WINSRC)/mess.rc $(WINUISRC)/mame32.rc $(WINUIOBJ)/mamevers32.rc
        GUIRESFILE = $(MESS_WINOBJ)/messgui.res
    else
        $(MESS_WINOBJ)/messcli.res: $(MESS_WINSRC)/mess.rc $(WINSRC)/mame.rc $(WINOBJ)/mamevers.rc
        CLIRESFILE = $(MESS_WINOBJ)/messcli.res
    endif
endif

$(LIBOSD): $(OSDOBJS)

OSDCOREOBJS += \
	$(OBJ)/mess/osd/windows/winmess.o	\
	$(OBJ)/mess/osd/windows/winutils.o	\
	$(OBJ)/mess/osd/windows/glob.o

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOCORE_NOMAIN): $(OSDCOREOBJS:$(WINOBJ)/main.o=)



#-------------------------------------------------
# generic rules for the resource compiler
#-------------------------------------------------

$(MESS_WINOBJ)/%.res: $(MESS_WINSRC)/%.rc
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) --include-dir mess/$(OSD) -o $@ -i $<

$(OBJ)/ui/%.res: src/ui/%.rc
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) --include-dir src/ui -o $@ -i $<

$(OBJ)/mess/ui/%.res: mess/ui/%.rc
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) --include-dir mess/ui --include-dir src/ui --include-dir src -o $@ -i $<
