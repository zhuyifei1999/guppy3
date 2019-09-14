class GSL_Error(Exception):
    pass


class TooManyErrors(GSL_Error):
    pass


class HadReportedError(GSL_Error):
    pass


class ReportedError(GSL_Error):
    pass


class UndefinedError(ReportedError):
    pass


class DuplicateError(ReportedError):
    pass


class CompositionError(GSL_Error):
    pass


class CoverageError(ReportedError):
    pass


class ConditionError(GSL_Error):
    pass
