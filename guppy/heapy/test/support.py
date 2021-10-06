"""Supporting definitions for the Heapy regression test.
Addapted from Python standard module test_support.
"""

import contextlib
import unittest
import pdb
import sys
import tracemalloc


class Error(Exception):
    """Base class for regression test exceptions."""


class TestFailed(Error):
    """Test failed."""


class TestSkipped(Error):
    """Test skipped.

    This can be raised to indicate that a test was deliberatly
    skipped, but not because a feature wasn't available.  For
    example, if some resource can't be used, such as the network
    appears to be unavailable, this should be raised instead of
    TestFailed.
    """


verbose = 1              # Flag set to 0 by regrtest.py
use_resources = None       # Flag set to [] by regrtest.py


# =======================================================================
# Preliminary PyUNIT integration.


class BasicTestRunner:
    def run(self, test):
        result = unittest.TestResult()
        test(result)
        return result


def run_suite(suite, testclass=None):
    """Run tests from a unittest.TestSuite-derived class."""
    if verbose:
        runner = unittest.TextTestRunner(sys.stdout, verbosity=2)
    else:
        runner = BasicTestRunner()

    result = runner.run(suite)
    if not result.wasSuccessful():
        if len(result.errors) == 1 and not result.failures:
            err = result.errors[0][1]
        elif len(result.failures) == 1 and not result.errors:
            err = result.failures[0][1]
        else:
            if testclass is None:
                msg = "errors occurred; run in verbose mode for details"
            else:
                msg = "errors occurred in %s.%s" \
                      % (testclass.__module__, testclass.__name__)
            raise TestFailed(msg)
        raise TestFailed(err)


def run_unittest(testclass, debug=0):
    """Run tests from a unittest.TestCase-derived class."""
    suite = unittest.makeSuite(testclass)
    if debug:
        suite.debug()
    else:
        run_suite(suite, testclass)


def debug_unittest(testclass):
    """ Debug tests from a unittest.TestCase-derived class."""
    run_unittest(testclass, debug=1)


# Base test case, tailored for heapy
class TestCase(unittest.TestCase):
    def setUp(self):
        from guppy import Root
        self.python = Root()
        self.guppy = self.python.guppy
        self.heapy = self.guppy.heapy
        self.Part = self.heapy.Part
        self.ImpSet = self.heapy.ImpSet
        self.Use = self.heapy.Use
        self.View = self.heapy.View
        self.iso = self.Use.iso
        self.idset = self.Use.idset
        self.version_info = sys.version_info

    def aseq(self, a, b, cont=0):
        if a != b:
            print("aseq: Expected: b = ", b)
            print("Got actually  : a = ", a)
            if cont <= 0:
                if cont < 0:
                    pdb.set_trace()
                else:
                    self.assertTrue(0)

    def asis(self, a, b, cont=0):
        if a is not b:
            print("asis: Expected: b = ", b)
            print("Got actually  : a = ", a)
            if cont <= 0:
                if cont < 0:
                    pdb.set_trace()
                else:
                    self.assertTrue(0)

    def tearDown(self):
        pass

    @contextlib.contextmanager
    def tracemalloc_state(enabled=True):
        orig_enabled = tracemalloc.is_tracing()

        def set_enabled(new_enabled):
            cur_enabled = tracemalloc.is_tracing()
            if cur_enabled == new_enabled:
                return

            if new_enabled:
                tracemalloc.start()
            else:
                tracemalloc.stop()

        set_enabled(enabled)

        try:
            yield
        finally:
            set_enabled(orig_enabled)
