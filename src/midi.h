#include <unistd.h>

inline void _midi_write(int fd, const void* buf, int size) {
  if (fd != -1) {
	if (write(fd, buf, size) == -1) {
	  g_warning("MIDI write to fd %d failed: %s", fd, strerror(errno));
	}
  }
}
