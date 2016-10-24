bin_PROGRAMS = tickq-to-archive

# We ship the git tree around because it includes version info, and is
# pretty small.
EXTRA_DIST = .git COPYING3 README.md

tickq_to_archive_CPPFLAGS = $(AMQ_CFLAGS) -std=c++11

tickq_to_archive_LDADD = -lcurl $(AMQ_LIBS) -ljson-c
tickq_to_archive_SOURCES = main.cc
