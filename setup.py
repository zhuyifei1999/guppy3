import os.path
import sys
from setuptools import setup, Extension
from distutils.command.install import INSTALL_SCHEMES

for scheme in INSTALL_SCHEMES.values():
    scheme['data'] = scheme['purelib']


setsc = Extension("guppy.sets.setsc", [
                      "src/sets/sets.c",
                      "src/sets/bitset.c",
                      "src/sets/nodeset.c"
                  ])

heapyc = Extension("guppy.heapy.heapyc", [
                       'src/heapy/heapyc.c',
                       'src/heapy/stdtypes.c'
                   ])


def doit():
    if sys.version_info.major < 3:
        print('''\
setup.py: Error: This guppy package only supports Python 3.
You can find the original Python 2 version, Guppy-PE, here:
http://guppy-pe.sourceforge.net/''', file=sys.stderr)
        sys.exit(1)
    if sys.implementation.name != 'cpython':
        print('''\
setup.py: Warning: This guppy package only supports CPython.
Compilation failure expected, but continuting anyways...''', file=sys.stderr)

    with open(os.path.join(os.path.dirname(__file__), 'README.md')) as f:
        long_description = f.read()

    setup(name="guppy3",
          version="3.1.0",
          description="Guppy 3 -- Guppy-PE ported to Python 3",
          long_description=long_description,
          long_description_content_type='text/markdown',
          author="YiFei Zhu",
          author_email="zhuyifei1999@gmail.com",
          url="https://github.com/zhuyifei1999/guppy3/",
          license='MIT',
          packages=[
              "guppy",
              "guppy.etc",
              "guppy.gsl",
              "guppy.heapy",
              "guppy.heapy.test",
              "guppy.sets",
          ],
          ext_modules=[setsc, heapyc],
          python_requires='>=3.5',
          classifiers=[
              "Programming Language :: Python :: 3",
              "Programming Language :: Python :: Implementation :: CPython",
              "Programming Language :: C",
              "License :: OSI Approved :: MIT License",
              "Operating System :: OS Independent",
              "Development Status :: 4 - Beta",
              "Topic :: Software Development :: Debuggers",
              "Environment :: Console",
              "Intended Audience :: Developers",
          ])


doit()
