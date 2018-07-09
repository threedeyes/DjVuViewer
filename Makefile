
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
		GoToWindow.o		

OBJDIR := Build

RDEF := Resource/resource.rdef

ifneq ($(shell uname -m),x86_64)
OBJS := $(OBJS) Render.o
endif

OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))

CC := g++
ASM := yasm
LD := $(CC)

LIBS := -lbe -ltranslation -ltracker -ldjvulibre
CFLAGS := -O3 -I./Includes
LDFLAGS := 

.PHONY : clean Build

default : Build

Build : $(BINARY)

$(BINARY) : $(OBJDIR) $(OBJS)
	$(LD) $(CFLAGS) $(OBJS) -o $(BINARY) $(LDFLAGS) $(LIBS)
	rc -o $(OBJDIR)/resource.rsrc $(RDEF)
	xres -o $(BINARY) $(OBJDIR)/resource.rsrc
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


