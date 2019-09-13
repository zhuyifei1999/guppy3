from guppy.heapy.test import support

PORTABLE_TEST = 1       # Relax tests to be more portable


class IdentityCase(support.TestCase):
    def test_1(self):
        import itertools
        import random
        import sys
        vs = list(range(100))
        random.shuffle(vs)
        vs = [float(i) for i in vs]
        x = self.iso(*vs).byid

        sz = sys.getsizeof(0.0)
        self.aseq(str(x)+'\n'+str(x.more)+'\n', """\
Set of 100 <float> objects. Total size = {sztotal} bytes.
 Index     Size   %   Cumulative  %   Value
     0      {sz:>3}   1.0      {szacc[0]:>4}   1.0 0.0
     1      {sz:>3}   1.0      {szacc[1]:>4}   2.0 1.0
     2      {sz:>3}   1.0      {szacc[2]:>4}   3.0 2.0
     3      {sz:>3}   1.0      {szacc[3]:>4}   4.0 3.0
     4      {sz:>3}   1.0      {szacc[4]:>4}   5.0 4.0
     5      {sz:>3}   1.0      {szacc[5]:>4}   6.0 5.0
     6      {sz:>3}   1.0      {szacc[6]:>4}   7.0 6.0
     7      {sz:>3}   1.0      {szacc[7]:>4}   8.0 7.0
     8      {sz:>3}   1.0      {szacc[8]:>4}   9.0 8.0
     9      {sz:>3}   1.0      {szacc[9]:>4}  10.0 9.0
<90 more rows. Type e.g. '_.more' to view.>
 Index     Size   %   Cumulative  %   Value
    10      {sz:>3}   1.0      {szacc[10]:>4}  11.0 10.0
    11      {sz:>3}   1.0      {szacc[11]:>4}  12.0 11.0
    12      {sz:>3}   1.0      {szacc[12]:>4}  13.0 12.0
    13      {sz:>3}   1.0      {szacc[13]:>4}  14.0 13.0
    14      {sz:>3}   1.0      {szacc[14]:>4}  15.0 14.0
    15      {sz:>3}   1.0      {szacc[15]:>4}  16.0 15.0
    16      {sz:>3}   1.0      {szacc[16]:>4}  17.0 16.0
    17      {sz:>3}   1.0      {szacc[17]:>4}  18.0 17.0
    18      {sz:>3}   1.0      {szacc[18]:>4}  19.0 18.0
    19      {sz:>3}   1.0      {szacc[19]:>4}  20.0 19.0
<80 more rows. Type e.g. '_.more' to view.>
""".format(sz=sz, sztotal=sz*100,
           szacc=list(itertools.accumulate([sz] * 20))))

    def test_2(self):
        # Slicing
        ss = []
        for i in range(100):
            for c in 'abc':
                ss.append(c*i)
        x = self.iso(*ss).byid

        def ae(x):
            lines = str(x).split('\n')
            datapos = lines[1].index('Representation')
            s = lines[2:]
            if s[-1].startswith('<'):
                s.pop()
            s = [line[datapos:] for line in s]
            return s

        def aeq(x, y):
            self.aseq(ae(x), ae(y))

        for i in range(0, 300, 60):
            b = x[i:]
            aeq(b, b.byid)

        # (B) in  Notes Aug 26 2005

        self.aseq(x.bysize[2].kind, x.bysize[2].bysize.kind)

    def test_3(self):
        # Some indexing cases.
        # Came up Sep 29 2005.
        # The kind of the result of indexing is to be
        # the result of the er of the partition.

        hp = self.Use

        x = hp.iso([], [], *list(range(20))).byid

        eq = [x[-10], x[-10:-9], x[12], x[12:13],
              x.parts[-10], x.parts[12]]
        k = x[-10].byid.kind
        for i in range(len(eq)):
            self.aseq(eq[i], eq[(i + 1) % len(eq)])
            self.aseq(eq[i].kind, eq[(i + 1) % len(eq)].kind)
            self.aseq(eq[i].kind, k)


class MixedCase(support.TestCase):
    def test_1(self):
        x = self.iso(1, 2, 1.0, 2.0, '1', '2')
        if not PORTABLE_TEST:
            self.aseq(str(x), """\
Partition of a set of 6 objects. Total size = 204 bytes.
 Index  Count   %     Size   % Cumulative  % Kind (class / dict of class)
     0      2  33      100  49       100  49 str
     1      2  33       56  27       156  76 int
     2      2  33       48  24       204 100 float""")

        for row in x.partition.get_rows():
            self.assertTrue(row.set <= row.kind)


class StatCase(support.TestCase):
    def test_1(self):
        hp = self.Use

        class C:
            pass
        c0 = C()

        class C:
            pass
        c1 = C()
        x = hp.iso(c0, c1)
        y = hp.iso(c1)

        d = x.diff(y)
        self.aseq(d.count, 1)
        self.aseq(d[0].count, 1)

        d = y.diff(x)
        self.aseq(d.count, -1)
        self.aseq(d[0].count, -1)

        d = x.diff(hp.iso())
        self.aseq(d.count, 2)
        self.aseq(d[0].count, 2)

        d = hp.iso().diff(x)
        self.aseq(d.count, -2)
        self.aseq(d[0].count, -2)


def test_main(debug=0):
    support.run_unittest(StatCase, debug)
    support.run_unittest(IdentityCase, debug)
    support.run_unittest(MixedCase, debug)


if __name__ == "__main__":
    test_main()
