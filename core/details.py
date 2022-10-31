MAJOR = 1
MINOR = 0
PATCH = 0
LOGO  = """\
 ::::::::  :::::::::      :::     :::    :::
:+:    :+: :+:    :+:   :+: :+:   :+:   :+:
+:+        +:+    +:+  +:+   +:+  +:+  +:+
+#+        +#++:++#+  +#++:++#++: +#++:++
+#+        +#+        +#+     +#+ +#+  +#+
#+#    #+# #+#        #+#     #+# #+#   #+#
 ########  ###        ###     ### ###    ###"""

VERSION     = f"v{MAJOR}.{MINOR}.{PATCH}".rjust(45)
DESCRIPTION = """
Configuration as code build tool for C/C++ designed to
make it easier to build your projects in every single way."""

LOGO_AND_VERSION = "\n".join([LOGO, VERSION])
