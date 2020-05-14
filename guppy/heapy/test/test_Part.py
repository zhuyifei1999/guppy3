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

        self.aseq(str(x.all)+'\n', """\
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
    20      {sz:>3}   1.0      {szacc[20]:>4}  21.0 20.0
    21      {sz:>3}   1.0      {szacc[21]:>4}  22.0 21.0
    22      {sz:>3}   1.0      {szacc[22]:>4}  23.0 22.0
    23      {sz:>3}   1.0      {szacc[23]:>4}  24.0 23.0
    24      {sz:>3}   1.0      {szacc[24]:>4}  25.0 24.0
    25      {sz:>3}   1.0      {szacc[25]:>4}  26.0 25.0
    26      {sz:>3}   1.0      {szacc[26]:>4}  27.0 26.0
    27      {sz:>3}   1.0      {szacc[27]:>4}  28.0 27.0
    28      {sz:>3}   1.0      {szacc[28]:>4}  29.0 28.0
    29      {sz:>3}   1.0      {szacc[29]:>4}  30.0 29.0
    30      {sz:>3}   1.0      {szacc[30]:>4}  31.0 30.0
    31      {sz:>3}   1.0      {szacc[31]:>4}  32.0 31.0
    32      {sz:>3}   1.0      {szacc[32]:>4}  33.0 32.0
    33      {sz:>3}   1.0      {szacc[33]:>4}  34.0 33.0
    34      {sz:>3}   1.0      {szacc[34]:>4}  35.0 34.0
    35      {sz:>3}   1.0      {szacc[35]:>4}  36.0 35.0
    36      {sz:>3}   1.0      {szacc[36]:>4}  37.0 36.0
    37      {sz:>3}   1.0      {szacc[37]:>4}  38.0 37.0
    38      {sz:>3}   1.0      {szacc[38]:>4}  39.0 38.0
    39      {sz:>3}   1.0      {szacc[39]:>4}  40.0 39.0
    40      {sz:>3}   1.0      {szacc[40]:>4}  41.0 40.0
    41      {sz:>3}   1.0      {szacc[41]:>4}  42.0 41.0
    42      {sz:>3}   1.0      {szacc[42]:>4}  43.0 42.0
    43      {sz:>3}   1.0      {szacc[43]:>4}  44.0 43.0
    44      {sz:>3}   1.0      {szacc[44]:>4}  45.0 44.0
    45      {sz:>3}   1.0      {szacc[45]:>4}  46.0 45.0
    46      {sz:>3}   1.0      {szacc[46]:>4}  47.0 46.0
    47      {sz:>3}   1.0      {szacc[47]:>4}  48.0 47.0
    48      {sz:>3}   1.0      {szacc[48]:>4}  49.0 48.0
    49      {sz:>3}   1.0      {szacc[49]:>4}  50.0 49.0
    50      {sz:>3}   1.0      {szacc[50]:>4}  51.0 50.0
    51      {sz:>3}   1.0      {szacc[51]:>4}  52.0 51.0
    52      {sz:>3}   1.0      {szacc[52]:>4}  53.0 52.0
    53      {sz:>3}   1.0      {szacc[53]:>4}  54.0 53.0
    54      {sz:>3}   1.0      {szacc[54]:>4}  55.0 54.0
    55      {sz:>3}   1.0      {szacc[55]:>4}  56.0 55.0
    56      {sz:>3}   1.0      {szacc[56]:>4}  57.0 56.0
    57      {sz:>3}   1.0      {szacc[57]:>4}  58.0 57.0
    58      {sz:>3}   1.0      {szacc[58]:>4}  59.0 58.0
    59      {sz:>3}   1.0      {szacc[59]:>4}  60.0 59.0
    60      {sz:>3}   1.0      {szacc[60]:>4}  61.0 60.0
    61      {sz:>3}   1.0      {szacc[61]:>4}  62.0 61.0
    62      {sz:>3}   1.0      {szacc[62]:>4}  63.0 62.0
    63      {sz:>3}   1.0      {szacc[63]:>4}  64.0 63.0
    64      {sz:>3}   1.0      {szacc[64]:>4}  65.0 64.0
    65      {sz:>3}   1.0      {szacc[65]:>4}  66.0 65.0
    66      {sz:>3}   1.0      {szacc[66]:>4}  67.0 66.0
    67      {sz:>3}   1.0      {szacc[67]:>4}  68.0 67.0
    68      {sz:>3}   1.0      {szacc[68]:>4}  69.0 68.0
    69      {sz:>3}   1.0      {szacc[69]:>4}  70.0 69.0
    70      {sz:>3}   1.0      {szacc[70]:>4}  71.0 70.0
    71      {sz:>3}   1.0      {szacc[71]:>4}  72.0 71.0
    72      {sz:>3}   1.0      {szacc[72]:>4}  73.0 72.0
    73      {sz:>3}   1.0      {szacc[73]:>4}  74.0 73.0
    74      {sz:>3}   1.0      {szacc[74]:>4}  75.0 74.0
    75      {sz:>3}   1.0      {szacc[75]:>4}  76.0 75.0
    76      {sz:>3}   1.0      {szacc[76]:>4}  77.0 76.0
    77      {sz:>3}   1.0      {szacc[77]:>4}  78.0 77.0
    78      {sz:>3}   1.0      {szacc[78]:>4}  79.0 78.0
    79      {sz:>3}   1.0      {szacc[79]:>4}  80.0 79.0
    80      {sz:>3}   1.0      {szacc[80]:>4}  81.0 80.0
    81      {sz:>3}   1.0      {szacc[81]:>4}  82.0 81.0
    82      {sz:>3}   1.0      {szacc[82]:>4}  83.0 82.0
    83      {sz:>3}   1.0      {szacc[83]:>4}  84.0 83.0
    84      {sz:>3}   1.0      {szacc[84]:>4}  85.0 84.0
    85      {sz:>3}   1.0      {szacc[85]:>4}  86.0 85.0
    86      {sz:>3}   1.0      {szacc[86]:>4}  87.0 86.0
    87      {sz:>3}   1.0      {szacc[87]:>4}  88.0 87.0
    88      {sz:>3}   1.0      {szacc[88]:>4}  89.0 88.0
    89      {sz:>3}   1.0      {szacc[89]:>4}  90.0 89.0
    90      {sz:>3}   1.0      {szacc[90]:>4}  91.0 90.0
    91      {sz:>3}   1.0      {szacc[91]:>4}  92.0 91.0
    92      {sz:>3}   1.0      {szacc[92]:>4}  93.0 92.0
    93      {sz:>3}   1.0      {szacc[93]:>4}  94.0 93.0
    94      {sz:>3}   1.0      {szacc[94]:>4}  95.0 94.0
    95      {sz:>3}   1.0      {szacc[95]:>4}  96.0 95.0
    96      {sz:>3}   1.0      {szacc[96]:>4}  97.0 96.0
    97      {sz:>3}   1.0      {szacc[97]:>4}  98.0 97.0
    98      {sz:>3}   1.0      {szacc[98]:>4}  99.0 98.0
    99      {sz:>3}   1.0      {szacc[99]:>4} 100.0 99.0
""".format(sz=sz, sztotal=sz*100,
           szacc=list(itertools.accumulate([sz] * 100))))

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
