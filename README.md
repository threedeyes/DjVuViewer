DjVuViewer
==========

DjVu Viewer for Haiku-OS

Based on djvulibre library - http://djvu.sourceforge.net/

Build
-----

Cloning repo:

    $ git clone https://github.com/threedeyes/DjVuViewer


Building djvulibre:

    $ cd djvulibre
    $ ./configure
    $ make
    $ cd ..

Building standalone DjVuViewer:

    $ make
    
Building system translator:

    $ make -f Makefile.Translator
    
Installing
----------

Installing DjVuViewer application:

    Copy DjVuViewer binary and Languages and Themes into your application folder
    
Installing translator:

    cp ./DjVuTranslator /boot/home/config/add-ons/Translators
        or...
    cp ./DjVuTranslator /boot/common/add-ons/Translators
    
