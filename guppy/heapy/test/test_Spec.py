from guppy.heapy.test import support
import sys
import unittest


class TestCase(support.TestCase):
    pass


class FirstCase(TestCase):
    def test_1(self):
        Spec = self.heapy.Spec
        TestEnv = Spec.mkTestEnv(Spec._Specification_)

        TestEnv.test_contains(Spec)


if __name__ == "__main__":
    support.run_unittest(FirstCase, 1)
