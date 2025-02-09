import os
import subprocess

for roots, dirs, files in os.walk('.'):
    if not dirs:
        subprocess.run(["touch", ".placeholder"])



