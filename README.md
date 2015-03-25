DjVuViewer
==========

DjVu Viewer and Translator for Haiku

Based on djvulibre library - http://djvu.sourceforge.net

Building
--------

Cloning repo:

	$ git clone https://github.com/threedeyes/DjVuViewer

Installing dependencies:

	$ pkgman install djvulibre_devel

Building standalone DjVuViewer:

	$ make

Building system translator:

	$ make -f Makefile.Translator

Installing
----------

Installing DjVuViewer application:

	Copy DjVuViewer binary, Languages and Themes folders into your application folder

Installing translator:

	cp DjVuTranslator /boot/system/non-packaged/add-ons/Translators
		or...
	cp DjVuTranslator /boot/home/config/non-packaged/add-ons/Translators
