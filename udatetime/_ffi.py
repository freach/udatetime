from datetime import datetime
from glob import glob
import os.path
from cffi import FFI
from ._pure import TZFixedOffset

'''
Requirements:
pip install cffi
sudo apt-get install libffi-dev

Can use, but should not because slooooow...
'''

file_base = os.path.dirname(__file__)

ffi = FFI()
ffi.cdef('''
typedef struct {
    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int wday;
    char ok;
} date_struct;

typedef struct {
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
    unsigned int fraction;
    int offset;
    char ok;
} time_struct;

typedef struct {
    date_struct date;
    time_struct time;
    char ok;
} date_time_struct;

typedef struct {
    double (*get_time)(void);
    void (*parse_date)(char*, date_struct*);
    void (*parse_time)(char*, time_struct*);
    void (*parse_date_time)(char*, date_time_struct*);
    void (*timestamp_to_date_time)(double, date_time_struct*, int);
    void (*format_date_time)(date_time_struct*, char*);
    void (*utcnow)(date_time_struct*);
    void (*localnow)(date_time_struct*);
    int (*get_local_utc_offset)(void);
} RFC3999_CAPI;

RFC3999_CAPI CAPI;
''')

C = ffi.dlopen(glob('%s/../rfc3339*.so' % file_base)[0])


def new_date_time_struct():
    data = {
        'date': {
            'year': 0,
            'month': 0,
            'day': 0,
            'wday': 0,
            'ok': '\0',
        },
        'time': {
            'hour': 0,
            'minute': 0,
            'second': 0,
            'fraction': 0,
            'offset': 0,
            'ok': '\0',
        },
        'ok': '\0',
    }

    return ffi.new('date_time_struct *', data)


def check_date_time_struct(dt):
    if dt.ok != '\1':
        if dt.date.ok != '\1':
            raise ValueError("Invalid RFC3339 date-time string. Date invalid.")
        elif dt.time.ok != '\1':
            raise ValueError("Invalid RFC3339 date-time string. Time invalid.")
        else:
            raise ValueError("Not supposed to happen!")


# static void check_timestamp_platform_support(double timestamp) {
#     double diff = timestamp - (double)((time_t)timestamp);

#     if (diff <= -1.0 || diff >= 1.0) {
#         PyErr_SetString(
#             PyExc_ValueError, "timestamp out of range for platform time_t"
#         );
#     }
# }


def dtstruct_to_datetime_obj(dt):
    if dt.ok == '\1':
        return datetime(
            dt.date.year,
            dt.date.month,
            dt.date.day,
            dt.time.hour,
            dt.time.minute,
            dt.time.second,
            dt.time.fraction,
            TZFixedOffset(dt.time.offset),
        )

    raise ValueError('Invalid date_time_struct')


def datetime_obj_to_dtstruct(date_time):
    dt = new_date_time_struct()

    dt.date.year = date_time.year
    dt.date.month = date_time.month
    dt.date.day = date_time.day
    # dt.date.wday = date_time.weekday() + 1
    dt.date.ok = '\1'

    dt.time.hour = date_time.hour
    dt.time.minute = date_time.minute
    dt.time.second = date_time.second
    dt.time.fraction = date_time.microsecond
    dt.time.offset = 0
    dt.time.ok = '\1'

    # TODO: Support all tzinfo subclasses by calling utcoffset()
    if date_time.tzinfo is not None and\
            date_time.tzinfo.__class__ is TZFixedOffset:
        dt.time.offset = date_time.tzinfo.offset

    dt.ok = '\1'

    return dt


def utcnow():
    '''datetime aware object in UTC with current date and time.'''
    dt = new_date_time_struct()
    C.CAPI.utcnow(dt)
    return dtstruct_to_datetime_obj(dt)


def now():
    '''datetime aware object in local timezone with current date and time.'''
    dt = new_date_time_struct()
    C.CAPI.localnow(dt)
    return dtstruct_to_datetime_obj(dt)


def from_rfc3339_string(rfc3339_string):
    '''Parse RFC3339 compliant date-time string.'''
    dt = new_date_time_struct()
    c_rfc3339_string = ffi.new('char[]', rfc3339_string)

    C.CAPI.parse_date_time(c_rfc3339_string, dt)
    check_date_time_struct(dt)
    return dtstruct_to_datetime_obj(dt)


def to_rfc3339_string(date_time):
    '''Serialize date_time to RFC3339 compliant date-time string.'''

    if date_time and date_time.__class__ is not datetime:
        raise ValueError("Expected a datetime object.")

    dt = datetime_obj_to_dtstruct(date_time)
    datetime_string = ffi.new('char[]', '0' * 33)
    C.CAPI.format_date_time(dt, ffi.cast('char*', datetime_string))

    return ffi.string(datetime_string)


def from_timestamp(timestamp, tz=None):
    '''timestamp[, tz] -> tz's local time from POSIX timestamp.'''

    offset = C.CAPI.get_local_utc_offset()
    # check_timestamp_platform_support(timestamp);

    # TODO: Support all tzinfo subclasses by calling utcoffset()
    if tz:
        if type(tz) is TZFixedOffset:
            offset = tz.offset
        else:
            raise TypeError("tz must be of type TZFixedOffset.")

    dt = new_date_time_struct()
    C.CAPI.timestamp_to_date_time(timestamp, dt, offset)
    check_date_time_struct(dt)

    return dtstruct_to_datetime_obj(dt)


def from_utctimestamp(timestamp):
    '''timestamp -> UTC datetime from a POSIX timestamp (like time.time()).'''

    # check_timestamp_platform_support(timestamp);

    dt = new_date_time_struct()
    C.CAPI.timestamp_to_date_time(timestamp, dt, 0)
    check_date_time_struct(dt)

    return dtstruct_to_datetime_obj(dt)


def utcnow_to_string():
    '''Current UTC date and time RFC3339 compliant date-time string.'''
    dt = new_date_time_struct()
    C.CAPI.utcnow(dt)

    datetime_string = ffi.new('char[33]', '\0')
    C.CAPI.format_date_time(dt, datetime_string)

    return ffi.string(datetime_string)


def now_to_string():
    '''Local date and time RFC3339 compliant date-time string.'''
    dt = new_date_time_struct()
    C.CAPI.localnow(dt)

    datetime_string = ffi.new('char[33]', '\0')
    C.CAPI.format_date_time(dt, datetime_string)

    return ffi.string(datetime_string)
