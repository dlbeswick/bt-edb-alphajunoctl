ACLOCAL_AMFLAGS = -I m4

SRC = src/alphajunoctl.c src/generated/generated-genums.c src/voice.c
BUILT_SOURCES = src/generated/generated-genums.h src/generated/generated-genums.c
CLEANFILES = $(BUILT_SOURCES)

plugin_LTLIBRARIES = libalphajunoctl.la

libalphajunoctl_la_SOURCES = $(SRC)
libalphajunoctl_la_CFLAGS = $(DEBUG_CFLAGS) $(BASE_DEPS_CFLAGS) -I $(prefix)/include -std=c99 -Werror=all -Wno-error=unused-variable
libalphajunoctl_la_LDFLAGS = -module -avoid-version
#libalphajunoctl_la_LIBADD = any-extra-libs.la...

# Remove 'la' file as the generated lib isn't intended to be linked with others.
install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/libalphajunoctl*.la
#	mv $(DESTDIR)$(plugindir)/libalphajunoctl.so $(DESTDIR)$(plugindir)/libalphajunoctl.so
#	$(LN_S) -f Matilde_Tracker2.html  $(DESTDIR)$(userdocdir)/Matilde_Tracker.html

#userdoc_DATA = Matilde_Tracker2.html

# Auto-generate GObject enum definitions from src/genums.h
src/generated/generated-genums.h: src/genums.h src/generated-genums.h.in
	$(AM_V_GEN)$(GLIB_MKENUMS) \
		--template=$(top_srcdir)/src/generated-genums.h.in \
		--output=$@ \
		$(top_srcdir)/src/genums.h
	test -f $@ || { echo "ERROR, file not generated"; exit 1; }

src/generated/generated-genums.c: src/genums.h src/generated-genums.c.in src/generated/generated-genums.h
	pwd
	$(AM_V_GEN)$(GLIB_MKENUMS) \
		--template=$(top_srcdir)/src/generated-genums.c.in \
		--output=$@ \
		$(top_srcdir)/src/genums.h
	test -f $@ || { echo "ERROR, file not generated"; exit 1; }
