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

esac
