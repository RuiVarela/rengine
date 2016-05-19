# Try to find the PulseAudio library
#
# Once done this will define:
#
#  PULSEAUDIO_FOUND - system has the PulseAudio library
#  PULSEAUDIO_INCLUDE_DIR - the PulseAudio include directory
#  PULSEAUDIO_LIBRARY - the libraries needed to use PulseAudio
#  PULSEAUDIO_MAINLOOP_LIBRARY - the libraries needed to use PulsAudio Mailoop
  
if (PULSEAUDIO_INCLUDE_DIR AND PULSEAUDIO_LIBRARY AND PULSEAUDIO_MAINLOOP_LIBRARY)
   # Already in cache, be silent
   set(PULSEAUDIO_FIND_QUIETLY TRUE)
endif (PULSEAUDIO_INCLUDE_DIR AND PULSEAUDIO_LIBRARY AND PULSEAUDIO_MAINLOOP_LIBRARY)
  
 FIND_PATH(PULSEAUDIO_INCLUDE_DIR pulse/pulseaudio.h HINTS ${PC_PULSEAUDIO_INCLUDEDIR} ${PC_PULSEAUDIO_INCLUDE_DIRS})
 FIND_LIBRARY(PULSEAUDIO_LIBRARY NAMES pulse libpulse HINTS ${PC_PULSEAUDIO_LIBDIR} ${PC_PULSEAUDIO_LIBRARY_DIRS})
 FIND_LIBRARY(PULSEAUDIO_MAINLOOP_LIBRARY NAMES pulse-mainloop pulse-mainloop-glib libpulse-mainloop-glib HINTS {PC_PULSEAUDIO_LIBDIR} ${PC_PULSEAUDIO_LIBRARY_DIRS})
 
 if (PULSEAUDIO_INCLUDE_DIR AND PULSEAUDIO_LIBRARY)
    set(PULSEAUDIO_FOUND TRUE)
 else (PULSEAUDIO_INCLUDE_DIR AND PULSEAUDIO_LIBRARY)
    set(PULSEAUDIO_FOUND FALSE)
 endif (PULSEAUDIO_INCLUDE_DIR AND PULSEAUDIO_LIBRARY)
 
if (PULSEAUDIO_FOUND)
  if (NOT PULSEAUDIO_FIND_QUIETLY)
      message(STATUS "Found PulseAudio: ${PULSEAUDIO_LIBRARY}")
      if (PULSEAUDIO_MAINLOOP_LIBRARY)
           message(STATUS "Found PulseAudio Mainloop: ${PULSEAUDIO_MAINLOOP_LIBRARY}")
      else (PULSAUDIO_MAINLOOP_LIBRARY)
          message(STATUS "Could NOT find PulseAudio Mainloop Library")
      endif (PULSEAUDIO_MAINLOOP_LIBRARY)
   endif (NOT PULSEAUDIO_FIND_QUIETLY)
else (PULSEAUDIO_FOUND)
   message(STATUS "Could NOT find PulseAudio")
endif (PULSEAUDIO_FOUND)
 
mark_as_advanced(PULSEAUDIO_INCLUDE_DIR PULSEAUDIO_LIBRARY PULSEAUDIO_MAINLOOP_LIBRARY)