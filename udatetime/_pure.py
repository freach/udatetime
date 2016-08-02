from datetime import tzinfo, timedelta, datetime
import time

DATE_TIME_FORMAT = '%Y-%m-%dT%H:%M:%S.%f'

local_utc_offset = None


class TZFixedOffset(tzinfo):

    def __init__(self, offset):
        self.offset = offset

    def utcoffset(self, dt=None):
        return timedelta(seconds=self.offset * 60)

    def dst(self, dt=None):
        return timedelta(seconds=self.offset * 60)

    def tzname(self, dt=None):
        sign = '+'
        if self.offset < 0:
            sign = '-'

        return "%s%d:%d" % (sign, self.offset / 60, self.offset % 60)

    def __repr__(self):
        return self.tzname()


def _get_local_utc_offset():
    global local_utc_offset

    if local_utc_offset is None:
        ts = time.time()
        utc_now = datetime.utcfromtimestamp(ts)
        now = datetime.fromtimestamp(ts)
        local_utc_offset = (now - utc_now).total_seconds() / 60

    return local_utc_offset


def utcnow():
    '''datetime aware object in UTC with current date and time.'''
    now = datetime.utcnow()
    return now.replace(tzinfo=TZFixedOffset(0))


def now():
    '''datetime aware object in local timezone with current date and time.'''
    now = datetime.now()
    return now.replace(tzinfo=TZFixedOffset(_get_local_utc_offset()))


def from_rfc3339_string(rfc3339_string):
    '''Parse RFC3339 compliant date-time string.'''

    rfc3339_string = rfc3339_string.replace(' ', '').lower()

    if 't' not in rfc3339_string:
        raise ValueError(
            'Invalid RFC3339 string. Missing \'T\' date/time separator.'
        )

    (date, _, _time) = rfc3339_string.partition('t')

    if not date or not _time:
        raise ValueError('Invalid RFC3339 string.')

    try:
        (year, month, day) = date.split('-')
        year = int(year)
        month = int(month)
        day = int(day)
    except ValueError:
        raise ValueError('Invalid RFC3339 string. Invalid date.')

    try:
        (hour, minute, second) = _time[:8].split(':')
        hour = int(hour)
        minute = int(minute)
        second = int(second)
    except ValueError:
        raise ValueError('Invalid RFC3339 string. Invalid time.')

    usec = 0
    offset = None

    if len(_time) > 8:
        if _time[8] == '.':
            usec_buf = ''

            for c in _time[9:]:
                if c in ('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'):
                    usec_buf += c
                else:
                    break

            if len(usec_buf) not in (3, 6):
                raise ValueError('Invalid RFC3339 string. Invalid fractions.')

            usec = int(usec_buf)

            if len(usec_buf) == 3:
                usec = usec * 1000

            _time = _time[9 + len(usec_buf):]
        elif _time[8] == 'z':
            offset = 0
        else:
            _time = _time[8:]
    else:
        offset = 0

    if offset is None and (len(_time) == 0 or _time[0] == 'z'):
        offset = 0
    elif offset is None:
        if _time[0] not in ('+', '-'):
            raise ValueError('Invalid RFC3339 string. Expected timezone.')

        negative = True if _time[0] == '-' else False

        try:
            (off_hour, off_minute) = _time[1:].split(':')

            off_hour = int(off_hour)
            off_minute = int(off_minute)
        except ValueError:
            raise ValueError('Invalid RFC3339 string. Invalid timezone.')

        offset = (off_hour * 60) + off_minute

        if negative:
            offset = offset * -1

    return datetime(
        year, month, day, hour, minute, second, usec, TZFixedOffset(offset)
    )


def to_rfc3339_string(date_time):
    '''Serialize date_time to RFC3339 compliant date-time string.'''

    if date_time and date_time.__class__ is not datetime:
        raise ValueError("Expected a datetime object.")

    rfc3339 = date_time.strftime(DATE_TIME_FORMAT)

    # TODO: Support all tzinfo subclasses by calling utcoffset()
    if date_time.tzinfo is not None and\
            date_time.tzinfo.__class__ is TZFixedOffset:

        offset = date_time.tzinfo.offset
        sign = '+'

        if offset < 0:
            sign = '-'

        return rfc3339 + '%c%02d:%02d' % (sign, offset / 60, offset % 60)

    return rfc3339 + '+00:00'


def from_timestamp(timestamp, tz=None):
    '''timestamp[, tz] -> tz's local time from POSIX timestamp.'''
    return datetime.fromtimestamp(timestamp, tz)


def from_utctimestamp(timestamp):
    '''timestamp -> UTC datetime from a POSIX timestamp (like time.time()).'''
    return datetime.utcfromtimestamp(timestamp)


def utcnow_to_string():
    '''Current UTC date and time RFC3339 compliant date-time string.'''
    return to_rfc3339_string(utcnow())


def now_to_string():
    '''Local date and time RFC3339 compliant date-time string.'''
    return to_rfc3339_string(now())
