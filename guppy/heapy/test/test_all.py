import importlib
import sys

autotests = (
    'dependencies',
    'gsl',
    'Classifiers',
    'heapyc',
    'ER',
    'OutputHandling',
    'Part',
    'Path',
    'RefPat',
    'UniSet',
    'View',
)

# These are not tested here

others = (
    'menuleak',
    'sf',
)


def test_main(debug=False):
    for name in autotests:
        testname = 'guppy.heapy.test.test_'+name
        try:
            del sys.modules[testname]
        except KeyError:
            pass
        x = importlib.import_module(testname)
        print('imported:', testname)
        f = x.test_main
        f(debug=debug)
        del sys.modules[testname]


if __name__ == '__main__':
    test_main()
