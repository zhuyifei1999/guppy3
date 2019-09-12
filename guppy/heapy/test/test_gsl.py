# Test the gsl subpackage
# Ideally this should be a top level test.


def test_main(debug=0):
    from guppy import Root
    gsl = Root().guppy.gsl
    gsl.Document._test_main_()
    gsl.DottedTree.test_main()
    gsl.FileIO.set_test_mode()
    gsl.Filer._test_main_()
    gsl.Gsml._test_main_()
    gsl.Main._test_main_()
    gsl.SpecNodes.test_main()
    # gsl.Text.test()


if __name__ == "__main__":
    test_main()
