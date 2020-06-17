# Tests of equivalence relations.

# These are also tested by test_Classifiers.
# This is some more tests, tailored esp. to the user view.
# (test_Classifiers was so slow already, so I start over)

# o Intended to be exhaustive wrt all ER's defined
#
# o Intersection of ER's

from guppy.heapy.test import support
import inspect
import os.path


class TestCase(support.TestCase):
    pass


class FirstCase(TestCase):
    def test_1(self):
        hp = self.heapy.Use
        hp.reprefix = 'hp.'

        a = hp.iso(1, '', 'asdf', 3.4, 3.7, 2)

        ts = (hp.Type & hp.Size)
        k = ts[a]

        # From Sep 1-2 2005
        # (h&dict).by(hp.Id.dictof&hp.Size)._get_partition()
        # (h&dict).by((hp.Type&hp.Size).dictof&hp.Size)

        # These require with_referrers of refby/via classifier
        # after gc collect referrers graph will be empty
        # (h).by(hp.Module.refby.dictof)
        # (h).by(hp.Via.dictof)

        # How to construct RCS / refby

        # self.aseq(hp.Type.refby(int, list) , hp.Type.refby(list, int)

        class C:
            pass

        di = hp.iso(C.__dict__, [])

        # db = di.by('Rcs')
        # for i in (0, 1):
        #     rk = repr(db[i].kind)
        #     ek = eval(rk, {'hp': hp}, {})
        #     self.aseq(ek, db[i].kind)
        #     self.aseq(db & ek, db[i])

    def test_2(self):
        ' Systematically test all kind constructors: '
        # wrt repr and evaluation of repr

        hp = self.heapy.Use
        hp.reprefix = 'hp.'

        class C:
            pass

        class T(object):
            pass

        c = C()
        t = T()
        import sys

        for s in (
            'hp.Clodo(dictof=C)',
            'hp.Clodo(dictof=T)',
            'hp.Clodo(dictof=())',
            'hp.Clodo(C)',
            'hp.Clodo(T)',
            'hp.Id(id(c))',
            'hp.Module("sys")',

            'hp.Prod()',
            'hp.Prod("<string>")',
            'hp.Prod(T)',
            'hp.Prod("/foo.py", 1)',
            'hp.Prod("/foo.py", None)',
            'hp.Prod(None, 1)',

            'hp.Rcs(hp.Clodo.sokind(int)(dictof=C))',
            'hp.Size(hp.iso(c).indisize)',
            'hp.Size(hp.iso(C).indisize).dictof',

            'hp.Type(T)',
            'hp.Type(int)',

            'hp.Unity()',

            'hp.Via()',
            # Via is also specially tested below
        ):
            x = eval(s, {'hp': hp, 'C': C, 'T': T, 'c': c}, {})
            rx = repr(x)
            self.aseq(eval(rx, {'hp': hp}, {}), x)

        for i, s in enumerate((
            # Test Via construction.
            # One test for each relation kind defined in Path except IDENTITY and RELSRC.
            # In code order.
            "hp.Via('_.x')",
            "hp.Via('_[0]')",
            "hp.Via('_.keys()[0]')",
            "hp.Via('_->abc')",
            "hp.Via('_.__dict__.keys()[0]')",
            "hp.Via('_.f_locals[\"abc\"]')",
            "hp.Via('_.f_locals [\"abc\"]')",
            "hp.Via('_->f_valuestack[0]')",
            "hp.Via('list(_)[0]')",
        )):
            code = i + 1
            x = eval(s, {'hp': hp}, {})
            rel = list(x.arg)[0]
            self.aseq(rel.kind, code)
            rx = repr(x)
            self.aseq(eval(rx, {'hp': hp}, {}), x)

    def test_3(self):
        ' Test of dictof '
        # Test of dictof on something that requires memoization, i.e. Size, & (and)

        hp = self.heapy.Use

        class C:
            pass

        class T(object):
            # The test works only if sizes of objects of class C and T differ.
            # At first test, T() was 4 bytes smaller than C().
            # This might be brittle with different systems.
            # This is to make sure this diff gets significantly bigger:
            __slots__ = '__dict__', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'

        c = C()
        t = T()
        dn = {}
        isod = hp.iso(c.__dict__, t.__dict__, dn)
        for x in (
                t, c):
            X = x.__class__
            for k in (
                hp.Clodo(dictof=X),
                hp.Size(hp.iso(x).indisize).dictof,
                hp.iso(x).bysize.kind.dictof,
                hp.iso(x.__dict__).kind,
            ):
                self.aseq(isod & k,  hp.iso(x.__dict__))

        # Test no-owner selection
        for k in (
            hp.Nothing.dictof,
        ):
            self.aseq(isod & k, hp.iso(dn))

    def test_4(self):
        ' Test of via '
        # Esp. representation, construction

        class C:
            pass

        c = C()
        hp = self.heapy.Use

        isod = hp.iso(c.__dict__)

        x = isod.by('Via').kind
        self.aseq(repr(x), "hpy().Via('.__dict__')")

    def test_5(self):
        ' Non-systematic tests that came up around Sep 14 2005 '

        class C:
            pass

        c = C()
        d = {}
        cref = [c]
        cref.append(cref)
        c.cref = cref
        hp = self.heapy.Use
        hp.reprefix = 'hp.'

        # I thought these should be the same
        a = hp.iso(C, c, c.__dict__, d)
        b = hp.iso(C, c, c.__dict__, d)
        self.aseq(a, b)

        # This is a kind of nested refdby that has been a concern lately
        # -- how to represent it

        s = hp.iso(C.__dict__, C, c, c.__dict__,
                   d).by(hp.Clodo.refdby.refdby)

        for i in range(len(s)):
            a = s[i].kind
            ra = repr(a)
            era = eval(ra, {'hp': hp}, {})
            self.aseq(a, era)

            self.aseq(s & era,
                      s[i])

        import sys

        p = sys.path
        del sys

        s = hp.iso(p)
        x = s.by(hp.Module.dictof.refdby)
        self.aseq(s & eval(repr(x.kind), {'hp': hp}, {}), s)

    def test_6(self):
        ' Test of .refdby on all others '
        with self.tracemalloc_state():
            class C:
                pass

            c = C()
            d = {}
            cref = [c]
            cref.append(cref)
            c.cref = cref

            hp = self.heapy.Use
            hp.reprefix = 'hp.'

            import sys
            s = hp.iso(C.__dict__, C, c, c.__dict__, d, sys)

            for pre in (
                'Unity',
                'Clodo',
                'Id',
                'Module',
                'Prod',
                ('Rcs', 0),
                'Size',
                'Type',
                'Via'
            ):
                if isinstance(pre, tuple):
                    pre, level = pre
                else:
                    level = 1
                er = getattr(hp, pre)
                self.er_test(er, s, level)

    def er_test(self, er, set, level=1):
        # Tests what any eqv. rel. er should do
        hp = self.heapy.Use

        rer = repr(er)
        self.aseq(eval(rer, {'hp': hp}, {}), er)

        for s in (set,):
            sby = s.by(er)
            sk = sby.kind
            rsk = repr(sk)
            ske = eval(rsk, {'hp': hp}, {})

            self.aseq(ske, sk, -1)
            self.aseq(s & sk, s, -1)
            self.aseq(s & ske, s, -1)

        # That it can do .refdby

        er_refdby = er.refdby

        # That it can do .dictof

        er_dictof = er.dictof

        if level > 0:
            self.er_test(er_refdby, set, level - 1)
            self.er_test(er_dictof, set, level - 1)

    def test_7(self):
        ' Test of alternative sets w. biper '

        hp = self.heapy.Use

        class C:
            pass

        class D(C):
            pass

        class E(D):
            pass

        class T(object):
            pass

        class U(T):
            pass

        class V(U):
            pass

        c = C()
        d = D()
        e = E()
        t = T()
        u = U()
        v = V()

        s = hp.iso([], {}, c, d, e, t, u, v, d.__dict__)

        for k in (
            hp.Size(32),
            hp.Type(U),
        ):

            lt = k.alt('<')
            le = k.alt('<=')
            ge = k.alt('>=')
            gt = k.alt('>=')
            ne = k.alt('!=')
            assert (s & le) & (s & ge) == s & k

            for a in (lt, le, ge, gt, ne, le & ~k):
                s & a

        import gc

        gc.collect()

        a = s.by(k.alt('>=').biper)
        b = s.by(k.alt('>=').biper)
        self.assertTrue(hp.iso(d.__dict__) <= a[0])
        self.assertTrue(a == b)

        gc.collect()

        a = s.by(k.alt('<=').biper)
        b = s.by(k.alt('<=').biper)
        self.assertTrue(hp.iso(d.__dict__) <= a[0])
        self.assertTrue(a == b)

    def test_8(self):
        ' Test of findex and biper '
        # added Nov 3 2005

        hp = self.heapy.Use

        class C:
            pass
        c = C()
        li = []
        di = {}

        s = hp.iso(li, di, c, 1)

        for k, i in (
            (hp.Type(dict), 0),
            (hp.iso(c), 1),
            (hp.iso(c, li), 1),
        ):
            p = s.by(k.biper)
            self.aseq(p[i].kind.fam.classifier.kinds[0], k)

    def test_9(self):
        ' Test the subrelation relation '

        from guppy import hpy
        ernames = ['Clodo', 'Id', 'Idset', 'Module',
                   'Prod', 'Rcs', 'Size', 'Type',
                   'Unity']

        hp = hpy()

        ers = [(name, getattr(hp, name)) for name in ernames]
        ers.append(('Size&Type', hp.Size & hp.Type))

        from io import StringIO
        f = StringIO()
        print(''.ljust(10), end=' ', file=f)
        for b in ers:
            print(b[0].ljust(7), end=' ', file=f)
        print(file=f)

        for a in ers:
            print(a[0].ljust(10), end=' ', file=f)
            for b in ers:
                print(str((a[1] < b[1]))[:1].ljust(7), end=' ', file=f)
            print(file=f)
        self.aseq(f.getvalue(), """\
           Clodo   Id      Idset   Module  Prod    Rcs     Size    Type    Unity   Size&Type \n\
Clodo      F       F       F       F       F       F       F       T       T       F       \n\
Id         F       F       F       F       F       F       F       F       T       F       \n\
Idset      F       F       F       F       F       F       F       F       T       F       \n\
Module     F       F       F       F       F       F       F       F       T       F       \n\
Prod       F       F       F       F       F       F       F       F       T       F       \n\
Rcs        F       F       F       F       F       F       F       F       T       F       \n\
Size       F       F       F       F       F       F       F       F       T       F       \n\
Type       F       F       F       F       F       F       F       F       T       F       \n\
Unity      F       F       F       F       F       F       F       F       F       F       \n\
Size&Type  F       F       F       F       F       F       T       T       T       F       \n\
""")

    def test_10(self):
        ' Test of producer profile '
        hp = self.heapy.Use

        with self.tracemalloc_state():
            class C:
                pass

            file = inspect.currentframe().f_code.co_filename

            def lineno():
                return inspect.currentframe().f_back.f_lineno

            str_aloc = eval('C()', {'C': C}, {})
            test_aloc, test_aloc_line = C(), lineno()

            def subf():
                return C(), lineno()

            subf_aloc, subf_aloc_line = subf()

            isod = hp.iso(None, str_aloc, test_aloc, subf_aloc)

            self.aseq(isod & hp.Prod(), hp.iso(None))
            self.aseq(isod & hp.Prod('<string>', 1), hp.iso(str_aloc))
            self.aseq(isod & hp.Prod(file, test_aloc_line), hp.iso(test_aloc))
            self.aseq(isod & hp.Prod(file, subf_aloc_line), hp.iso(subf_aloc))

            self.aseq(isod & hp.Prod(file), hp.iso(test_aloc, subf_aloc))
            self.aseq(isod & hp.Prod(os.path.dirname(file)),
                      hp.iso(test_aloc, subf_aloc))

            self.aseq(isod & hp.Prod(self.test_10), hp.iso(test_aloc, subf_aloc))
            self.aseq(isod & hp.Prod(subf), hp.iso(subf_aloc))

            self.aseq(isod & hp.Prod(None, test_aloc_line).alt('>=') &
                      hp.Prod(None, test_aloc_line).alt('<='),
                      hp.iso(test_aloc))

        with self.assertRaises(TypeError):
            hp.Prod('', lineno=1)

        with self.assertRaises(TypeError):
            hp.Prod(filename='', line=1)

        with self.assertRaises(TypeError):
            hp.Prod(None)

        with self.assertRaises(TypeError):
            hp.Prod(1, 1)

        with self.assertRaises(TypeError):
            hp.Prod('', '')

        with self.assertRaises(TypeError):
            hp.Prod('', 1, 2)


def test_main(debug=0):
    support.run_unittest(FirstCase, debug)


if __name__ == "__main__":
    from guppy.heapy.Remote import off
    off()

    test_main()
