# LuaBridge is published under the terms of the MIT License.

# The original version of LuaBridge was written by Nathan Reed. The project has been taken over by Vinnie Falco, who added new functionality, wrote the new documentation, and incorporated contributions from Nigel Atkinson.
# For questions, comments, or bug reports feel free to open a Github issue or contact Vinnie Falco directly at the email address indicated below.

# Copyright 2012, Vinnie Falco (<vinnie.falco@gmail.com>)
# Copyright 2008, Nigel Atkinson
# Copyright 2007, Nathan Reed

# Portions from The Loki Library:
# Copyright (C) 2001 by Andrei Alexandrescu

# Older versions of LuaBridge up to and including 0.2 are distributed under the BSD 3-Clause License. See the corresponding license file in those versions (distributed separately) for more details.

# NOTE: This file Added by Nous Xiong for working properly with biicode

if (BIICODE)
  # Initializes block variables.
  INIT_BIICODE_BLOCK()
  
  set (BII_LIB_TYPE STATIC)

  # Actually create targets: EXEcutables and libraries.
  ADD_BIICODE_TARGETS()
endif ()
