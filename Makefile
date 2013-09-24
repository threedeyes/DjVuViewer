
BINARY := DjVuViewer

OBJS := MainApp.o \
		MainWindow.o \
		AboutWindow.o \
		Settings.o \
		PicButton.o \
		Theme.o \
		Label.o \
		Language.o \
		ConfigureWindow.o \
		IconMenuItem.o \
		IconListItem.o \
		ToolsContainer.o \
		ShowImageView.o \
		Filter.o \
		LoadDjVu.o \
		ThumbListItem.o \
		GoToWindow.o \
		Render.o

OBJDIR := Build

RSRCS := Resource/resource.rsrc

OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))

CC := g++
ASM := yasm
LD := $(CC)

LIBS := -lroot -lbe -ltranslation -ltracker -ljpeg -L./djvulibre/libdjvu/.libs -ldjvulibre
CFLAGS := -O3 -I./Includes -I./djvulibre
LDFLAGS := 

.PHONY : clean Build

default : Build

Build : $(BINARY)
	
$(BINARY) : $(OBJDIR) $(OBJS) $(RSRCS)
	$(LD) $(CFLAGS) $(OBJS) -o $(BINARY) $(LDFLAGS) $(LIBS)
	xres -o $(BINARY) $(RSRCS)  Themes/Default
	mimeset -f $(BINARY)

clean:
	rm -rf $(OBJDIR)/*.*
	rm $(BINARY)

Build/%.o : Src/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

Build/%.o : Src/%.asm
	@mkdir -p $(OBJDIR)
	$(ASM) -f elf $< -o $@


