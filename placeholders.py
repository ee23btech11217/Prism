import os
import subprocess

for roots, dirs, files in os.walk('.'):
    if not dirs and not files:
        path = os.path.join(roots, ".placeholder")

        with open(path, 'w') as pl:
            pl.write("Adding folder structure to git")
        print(roots)

