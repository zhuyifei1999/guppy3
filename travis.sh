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
        pip install coverage
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
        bash <(curl -s https://codecov.io/bash)
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
        pip install cibuildwheel==1.8.0
        ;;
    script)
        CIBW_BUILD='cp3[6789]-*' cibuildwheel --output-dir wheelhouse
        ;;
    after_success)
        pip install twine
        TWINE_USERNAME=__token__ twine upload --skip-existing wheelhouse/*.whl
        ;;
    esac
    ;;

esac
