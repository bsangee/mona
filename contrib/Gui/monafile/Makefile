TARGET  = MONAFILE
SOURCES = FileManager.cpp FileWindow.cpp FileBrowser.cpp Icon.cpp
INSTDIR = $(BINDIR)/APPS/MONAFRMS/$(TARGET).APP
INSTFILES  = ICONS.BMP $(TARGET).map
CLEANFILES =

SHAREDIR   = $(MONADIR)/share
ifneq ($(BUILD_TARGET),ELF)
ADDLINK    = -lgui-imp --enable-auto-import
ADDLINKDEP = $(MONADIR)/lib/libgui-imp.a
include $(SHAREDIR)/configs/monapi-ex5.inc
else
ADDLINK    = -lgui
ADDLINKDEP = $(MONADIR)/lib/libgui.a
include $(SHAREDIR)/configs/monapi-el5.inc
endif

CXXFLAGS += -DDEBUG

$(TARGET).EXE: $(LIBDIR)/libgui.a

ICONS.BM5: icons.bmp
	`which bim2bin` in:$< out:$@ -osacmp -tek5 BS:0 eprm:z0

-include dependencies
