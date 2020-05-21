#! /bin/bash

set -ex

case "$ACTION" in
test)
    case "$1" in
    install)
        pip install -ve .
        ;;
    script)
        $PYTHON -c '__import__("faulthandler").enable(); __import__("guppy").hpy().test(); __import__("guppy").heapy.heapyc.xmemstats()'
        ;;
    esac
    ;;

tracemalloctest)
    case "$1" in
    install)
        pip install -ve .
        ;;
    script)
        $PYTHON -X tracemalloc -c '__import__("faulthandler").enable(); __import__("guppy").hpy().test(); __import__("guppy").heapy.heapyc.xmemstats()'
        ;;
    esac
    ;;

sdisttest)
    case "$1" in
    install)
        $PYTHON setup.py sdist --formats=gztar
        DISTFILE="$(realpath "dist/$($PYTHON setup.py --fullname).tar.gz")"
        $PYTHON -m venv venv
        source venv/bin/activate
        pushd /tmp
        pip install -v "$DISTFILE"
        popd
        deactivate
        ;;
    script)
        source venv/bin/activate
        pushd /tmp
        python -c '__import__("faulthandler").enable(); __import__("guppy").hpy().test(); __import__("guppy").heapy.heapyc.xmemstats()'
        popd
        deactivate
        ;;
    esac
    ;;

codecov)
    case "$1" in
    install)
        pip install coverage codecov
        # https://github.com/codecov/codecov-python/issues/250
        patch "$(python -c 'import codecov; print(codecov.__file__)')" << 'EOF'
diff --git a/codecov/__init__.py b/codecov/__init__.py
index 2a2a3f6..a0c724c 100644
--- a/codecov/__init__.py
+++ b/codecov/__init__.py
@@ -4,6 +4,7 @@ import os
 import re
 import sys
 import glob
+import itertools
 import requests
 import argparse
 from time import sleep
@@ -869,21 +870,27 @@ def main(*argv, **kwargs):
             write("XX> Skip processing gcov")
 
         else:
-            dont_search_here = (
-                "-not -path './bower_components/**' "
-                "-not -path './node_modules/**' "
-                "-not -path './vendor/**'"
-            )
+            dont_search_here = [
+                "-not",
+                "-path",
+                "./bower_components/**",
+                "-not",
+                "-path",
+                "./node_modules/**",
+                "-not",
+                "-path",
+                "./vendor/**",
+            ]
             write("==> Processing gcov (disable by -X gcov)")
             cmd = [
                 "find",
                 (sanitize_arg("", codecov.gcov_root or root)),
-                dont_search_here,
+                *dont_search_here,
                 "-type",
                 "f",
                 "-name",
                 "*.gcno",
-                " ".join(map(lambda a: "-not -path '%s'" % a, codecov.gcov_glob)),
+                *itertools.chain(*(["-not", "-path", a] for a in codecov.gcov_glob)),
                 "-exec",
                 (sanitize_arg("", codecov.gcov_exec or "")),
                 "-pb",
EOF
        CFLAGS=-coverage pip install -ve .
        ;;
    script)
        TESTPY="$(mktemp XXXXXXXXXX.py)"
        cat > "$TESTPY" << EOF
from guppy import hpy, heapy
hpy().test()
heapy.heapyc.xmemstats()
EOF
        coverage run "$TESTPY"
        ;;
    after_success)
        codecov
        ;;
    esac
    ;;

cpychecker)
    case "$1" in
    install)
        pip install six
        pushd /tmp
        git clone --branch v0.17 https://github.com/davidmalcolm/gcc-python-plugin.git gcc-python-plugin
        cd gcc-python-plugin
        CC=gcc-6 CXX=g++-6 make plugin
        popd
        ;;
    script)
        # Not ready yet, permit failure
        CC_FOR_CPYCHECKER=gcc-6 CC=/tmp/gcc-python-plugin/gcc-with-cpychecker CFLAGS=--cpychecker-verbose pip install -ve . || true
        ;;
    esac
    ;;

cibuildwheel-source)
    case "$1" in
    install)
        ;;
    script)
        $PYTHON setup.py sdist --formats=gztar
        ;;
    after_success)
        pip install twine
        TWINE_USERNAME=__token__ twine upload --skip-existing dist/*.tar.gz
        ;;
    esac
    ;;
cibuildwheel)
    case "$1" in
    install)
        pip install cibuildwheel==1.4.1
        ;;
    script)
        CIBW_BUILD='cp3[5678]-*' cibuildwheel --output-dir wheelhouse
        ;;
    after_success)
        pip install twine
        TWINE_USERNAME=__token__ twine upload --skip-existing wheelhouse/*.whl
        ;;
    esac
    ;;

esac
