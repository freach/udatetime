#include <Python.h>
#include <datetime.h>
#include <structmember.h>

#define RFC3339_VERSION "0.0.2"
#define DAY_IN_SECS 86400
#define HOUR_IN_SECS 3600
#define MINUTE_IN_SECS 60
#define HOUR_IN_MINS 60

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
    int offset; // UTC offset in minutes
    char ok;
} time_struct;

typedef struct {
    date_struct date;
    time_struct time;
    char ok;
} date_time_struct;

static int local_utc_offset; // local's system offset to UTC, init later

/*
 * Remove space characters from source
 */
static void _strip_spaces(char *source) {
    char *i = source;

    while(*source != 0) {
        *i = *source++;
        if (*i != ' ') i++;
    }

    *i = 0;
}

/* Get the local time zone's offset to UTC
 *
 * Uses tm_gmtoff in tm struct, which requires a POSIX system.
 * TODO: Cross platform compatibility
 */
static int _get_local_utc_offset(void) {
    struct tm *info;
    time_t n = time(NULL);
    info = localtime(&n);

    // tm_gmtoff requires POSIX
    return (int)(*info).tm_gmtoff / HOUR_IN_MINS;
}

/* Get current time using gettimeofday(), ftime() or time() depending on
 * support.
 */
static double gettime(void) {
#if defined(HAVE_GETTIMEOFDAY)
    // => Use gettimeofday() in usec
    struct timeval t;
#if defined(GETTIMEOFDAY_NO_TZ)
    if (gettimeofday(&t) == 0)
        return ((double)t.tv_sec) + ((double)t.tv_usec * 0.000001);
#else
    struct timezone *tz = NULL;
    if (gettimeofday(&t, tz) == 0)
        return ((double)t.tv_sec) + ((double)t.tv_usec * 0.000001);
#endif
#elif defined(HAVE_FTIME)
    // => Use ftime() in msec
    struct timeb t;
    ftime(&t);
    return ((double)t.time) + ((double)t.millitm * 0.001);
#else
    // Fallback to time() in sec
    time_t t;
    time(&t);
    return (double)t;
#endif

    // -Wreturn-type
    return 0.0;
}

/*
 * Parse a RFC3339 full-date
 * full-date = date-fullyear "-" date-month "-" date-mday
 * Ex. 2007-08-31
 *
 * Characters after date-mday are being ignored, so you can pass a
 * date-time string and parse out only the full-date part.
 */
static void _parse_date(char *date_string, date_struct *d) {
    // operate on string copy
    char* const tokens = strdup(date_string);

    // remove spaces
    _strip_spaces(tokens);

    // invalidate date_struct
    (*d).ok = 0;

    if (strlen(tokens) < 10)
        return;

    int status = sscanf(
        tokens, "%04d-%02d-%02d", &((*d).year), &((*d).month), &((*d).day)
    );

    // Validate parsed tokens
    if (status != 3) return;
    if ((*d).year < 1 || (*d).year > 9999) return;
    if ((*d).month < 1 || (*d).month > 12) return;
    if ((*d).day < 1 || (*d).day > 31) return;

    unsigned int leap = ((*d).year % 4 == 0) &&\
        ((*d).year % 100 || ((*d).year % 400 == 0));

    // Validate max day based on month
    switch((*d).month) {
        case 1:
            if ((*d).day > 31)
                return;
            break;
        case 2:
            if (leap > 0) {
                if ((*d).day > 29)
                    return;
            } else {
                if ((*d).day > 28)
                    return;
            }
            break;
        case 3:
            if ((*d).day > 31)
                return;
            break;
        case 4:
            if ((*d).day > 30)
                return;
            break;
        case 5:
            if ((*d).day > 31)
                return;
            break;
        case 6:
            if ((*d).day > 30)
                return;
            break;
        case 7:
            if ((*d).day > 31)
                return;
            break;
        case 8:
            if ((*d).day > 31)
                return;
            break;
        case 9:
            if ((*d).day > 30)
                return;
            break;
        case 10:
            if ((*d).day > 31)
                return;
            break;
        case 11:
            if ((*d).day > 30)
                return;
            break;
        case 12:
            if ((*d).day > 31)
                return;
            break;
    }

    (*d).ok = 1;
}

/*
 * Parse a RFC3339 partial-time or full-time
 * partial-time = time-hour ":" time-minute ":" time-second [time-secfrac]
 * full-time    = partial-time time-offset
 * Ex. 16:47:31.123+00:00, 18:21:00.123, 18:21:00
 *
 * If time_string is partial-time timezone will be UTC.
 * If time_string is date-time, full-date part will be ignored.
 */
static void _parse_time(char *time_string, time_struct *t) {
    // operate on string copy
    char *tokens = strdup(time_string);
    char *token_ptr = tokens;

    // remove spaces
    _strip_spaces(tokens);

    // invalidate time_struct
    (*t).ok = 0;

    // must be at least hh:mm:ss, no timezone implicates UTC
    if (strlen(tokens) < 8)
        goto cleanup;

    // check if time_string is date-time string, for convenience reasons
    if ((strlen(tokens) > 11) && ((*(tokens + 10 ) == 'T') || (*(tokens + 10 ) == 't'))) {
        tokens += 11;
    }

    int status = sscanf(
        tokens, "%02d:%02d:%02d", &((*t).hour), &((*t).minute), &((*t).second)
    );

    // Validate parsed tokens
    if (status != 3) goto cleanup;
    if ((*t).hour < 0 || (*t).hour > 23) goto cleanup;
    if ((*t).minute < 0 || (*t).minute > 59) goto cleanup;
    if ((*t).second < 0 || (*t).second > 59) goto cleanup;

    // dealt with hh:mm:ss
    if (strlen(tokens) == 8) {
        (*t).offset = 0;
        (*t).ok = 1;
        goto cleanup;
    } else {
        tokens += 8;
    }

    // check for fractions
    if (*tokens == '.') {
        tokens++;
        char fractions[6] = {0};

        // Substring fractions, max 6 digits for usec
        for (unsigned int i = 0; i < 6; i++) {
            if ((*(tokens + i) >= 48) && (*(tokens + i) <= 57)) {
                fractions[i] = *(tokens + i);
            } else {
                break;
            }
        }

        // convert fractions to uint
        status = sscanf(fractions, "%d", &((*t).fraction));

        if (strlen(fractions) == 3) {
            (*t).fraction = (*t).fraction * 1000; // convert msec to usec
        } else if (strlen(fractions) == 6) {
            // all fine, already in usec
        } else {
            goto cleanup; // Invalid fractions must be msec or usec
        }

        // validate
        if (status != 1) goto cleanup;
        if ((*t).fraction < 0 || (*t).fraction > 999999) goto cleanup;

        tokens += strlen(fractions);

        // no timezone provided
        if (strlen(tokens) == 0) {
            (*t).offset = 0;
            (*t).ok = 1;
            goto cleanup;
        }
    }

    // parse timezone
    if ((*tokens == 'Z') || (*tokens == 'z')) {
        (*t).offset = 0;
        (*t).ok = 1;
        goto cleanup;
    } else if ((*tokens == '+') || (*tokens == '-')) {
        unsigned int tz_hour, tz_minute;

        status = sscanf(tokens + 1, "%02d:%02d", &tz_hour, &tz_minute);

        // validate
        if (status != 2) goto cleanup;
        if ((tz_hour < 0) || (tz_hour > 23)) goto cleanup;
        if ((tz_minute < 0) || (tz_minute > 59)) goto cleanup;

        // final offset
        int tz_offset = (tz_hour * HOUR_IN_MINS) + tz_minute;

        // make final offset negative
        if (*tokens == '-') {
            tz_offset = tz_offset * -1;
        }

        (*t).offset = tz_offset;
        (*t).ok = 1;
    }

cleanup:
    free(token_ptr);
    tokens = NULL;
    token_ptr = NULL;
    return;
}

/*
 * Parse a RFC3339 date-time
 * date-time = full-date "T" full-time
 * Ex. 2007-08-31T16:47:31+00:00 or 2007-12-24T18:21:00.123Z
 *
 * Using " " instead of "T" is NOT supported.
 */
static void _parse_date_time(char *datetime_string, date_time_struct *dt) {
    _parse_date(datetime_string, &((*dt).date));
    if ((*dt).date.ok == 0)
        return;

    _parse_time(datetime_string, &((*dt).time));
    if ((*dt).time.ok == 0)
        return;

    (*dt).ok = 1;
}

/*
 * Create date-time with current values (time now) with given timezone offset
 * offset = UTC offset in minutes
 */
static void _now(date_time_struct* now, int offset) {
    unsigned int n, sec, min, hour, mday, mon, year, wday, days, leap, yday;

    double t = gettime();
    unsigned int tsec = (unsigned int)t;
    unsigned int tusec = (unsigned int)((t - (double)tsec) * 1000000);

    n = tsec + (offset * MINUTE_IN_SECS);
    days = n / DAY_IN_SECS;
    wday = (4 + days) % 7;

    n %= DAY_IN_SECS;
    hour = n / HOUR_IN_SECS;
    n %= HOUR_IN_SECS;
    min = n / HOUR_IN_MINS;
    sec = n % MINUTE_IN_SECS;

    days = days + 719468;
    year = (days + 2) * 400 / 146097;
    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if (yday < 0) {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    mon = (yday + 31) * 10 / 306;
    mday = yday - (367 * mon / 12 - 30) + 1;

    if (yday >= 306) {
        year++;
        mon -= 10;
    } else {
        mon += 2;
    }

    (*now).date.year = year;
    (*now).date.month = mon;
    (*now).date.day = mday;
    (*now).date.wday = (wday) + 1;

    (*now).time.hour = hour;
    (*now).time.minute = min;
    (*now).time.second = sec;
    (*now).time.fraction = tusec; // sec fractions in microseconds
    (*now).time.offset = offset;

    (*now).ok = 1;
}

/*
 * Create RFC3339 date-time string
 */
static void _format_date_time(date_time_struct *dt, char* datetime_string) {
    char sign = '+';
    if ((*dt).time.offset < 0) sign = '-';

    sprintf(
        datetime_string,
        "%04d-%02d-%02dT%02d:%02d:%02d.%06d%c%02d:%02d",
        (*dt).date.year,
        (*dt).date.month,
        (*dt).date.day,
        (*dt).time.hour,
        (*dt).time.minute,
        (*dt).time.second,
        (*dt).time.fraction,
        sign,
        (*dt).time.offset / HOUR_IN_MINS,
        (*dt).time.offset % HOUR_IN_MINS
    );
}

/*
 * Create date-time with current values in UTC
 */
#define _utcnow(now) _now(now, 0)

/*
 * Create date-time with current values in systems local timezone
 */
static void _localnow(date_time_struct* now) {
    if (!local_utc_offset)
        local_utc_offset = _get_local_utc_offset();
    return _now(now, local_utc_offset);
}


/*
 * ***======================= CPython Section =======================***
 */

/*
 * class FixedOffset(tzinfo):
 */
typedef struct {
    PyObject_HEAD
    int offset;
} FixedOffset;

/**
 * def __init__(self, offset):
 *     self.offset = offset
 */
static int FixedOffset_init(FixedOffset *self, PyObject *args, PyObject *kwargs) {
    int offset;
    if (!PyArg_ParseTuple(args, "i", &offset))
        return -1;

    self->offset = offset;
    return 0;
}

/**
 * def utcoffset(self, dt):
 *     return timedelta(seconds=self.offset * 60)
 */
static PyObject *FixedOffset_utcoffset(FixedOffset *self, PyObject *args) {
    return PyDelta_FromDSU(0, self->offset * 60, 0);
}

/**
 * def dst(self, dt):
 *     return timedelta(seconds=self.offset * 60)
 */
static PyObject *FixedOffset_dst(FixedOffset *self, PyObject *args) {
    return PyDelta_FromDSU(0, self->offset * 60, 0);
}

/**
 * def tzname(self, dt):
 *     sign = '+'
 *     if self.offset < 0:
 *         sign = '-'
 *     return "%s%d:%d" % (sign, self.offset / 60, self.offset % 60)
 */
static PyObject *FixedOffset_tzname(FixedOffset *self, PyObject *args) {
    char tzname[7] = {0};
    char sign = '+';

    if (self->offset < 0) sign = '-';

    sprintf(
        tzname,
        "%c%02d:%02d",
        sign,
        self->offset / HOUR_IN_MINS,
        self->offset % HOUR_IN_MINS
    );

    return PyString_FromString(tzname);
}

/**
 * def __repr__(self):
 *     return self.tzname()
 */
static PyObject *FixedOffset_repr(FixedOffset *self) {
    return FixedOffset_tzname(self, NULL);
}

static void FixedOffset_dealloc(FixedOffset *self) {
    self->ob_type->tp_free((PyObject*)self);
}

static PyMemberDef FixedOffset_members[] = {
    {"offset", T_INT, offsetof(FixedOffset, offset), 0, "UTC offset"},
    {NULL}
};

static PyMethodDef FixedOffset_methods[] = {
    {"utcoffset", (PyCFunction)FixedOffset_utcoffset, METH_VARARGS, ""},
    {"dst",       (PyCFunction)FixedOffset_dst,       METH_VARARGS, ""},
    {"tzname",    (PyCFunction)FixedOffset_tzname,    METH_VARARGS, ""},
    {NULL}
};

static PyTypeObject FixedOffset_type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "rfc3339.FixedOffset_type", /*tp_name*/
    sizeof(FixedOffset),       /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)FixedOffset_dealloc,/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    (reprfunc)FixedOffset_repr,/*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc)FixedOffset_repr,/*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,       /*tp_flags*/
    "TZInfo with fixed offset",/* tp_doc */
};

/*
 * Instantiate new FixedOffset_type object
 * Skip overhead of calling PyObject_New and PyObject_Init.
 * Directly allocate object.
 */
static PyObject *new_fixed_offset_ex(int offset, PyTypeObject *type) {
    FixedOffset *self = (FixedOffset *) (type->tp_alloc(type, 0));

    if (self != NULL)
        self->offset = offset;

    return (PyObject *) self;
}

#define new_fixed_offset(offset) new_fixed_offset_ex(offset, &FixedOffset_type)

static PyObject *dtstruct_to_datetime_obj(date_time_struct *dt) {
    if ((*dt).ok == 1) {
        PyObject *new_datetime = PyDateTimeAPI->DateTime_FromDateAndTime(
            (*dt).date.year,
            (*dt).date.month,
            (*dt).date.day,
            (*dt).time.hour,
            (*dt).time.minute,
            (*dt).time.second,
            (*dt).time.fraction,
            new_fixed_offset((*dt).time.offset),
            PyDateTimeAPI->DateTimeType
        );

        if (PyErr_Occurred())
            return NULL;

        return new_datetime;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *utcnow(PyObject *self) {
    date_time_struct dt = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, 0};
    _utcnow(&dt);
    return dtstruct_to_datetime_obj(&dt);
}

static PyObject *localnow(PyObject *self) {
    date_time_struct dt = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, 0};
    _localnow(&dt);
    return dtstruct_to_datetime_obj(&dt);
}

static PyObject *from_rfc3339_string(PyObject *self, PyObject *args) {
    char *rfc3339_string;

    if (!PyArg_ParseTuple(args, "s", &rfc3339_string))
        return NULL;

    date_time_struct dt = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, 0};
    _parse_date_time(rfc3339_string, &dt);

    if (dt.ok != 1) {
        if (dt.date.ok != 1) {
            PyErr_Format(
                PyExc_ValueError,
                "Invalid RFC3339 date-time string. Date invalid."
            );
        } else if (dt.time.ok != 1) {
            PyErr_Format(
                PyExc_ValueError,
                "Invalid RFC3339 date-time string. Time invalid."
            );
        } else {
            PyErr_Format(PyExc_ValueError, "Not supposed to happen!");
        }

        return NULL;
    }

    PyObject *res = dtstruct_to_datetime_obj(&dt);

    return res;
}

static PyObject *to_rfc3339_string(PyObject *self, PyObject *args) {
    PyObject *obj = NULL;

    if (!PyArg_ParseTuple(args, "O", &obj))
        return NULL;

    if (!PyDateTime_Check(obj)) {
        PyErr_SetString(PyExc_ValueError, "Expected a datetime object.");
        return NULL;
    }

    PyDateTime_DateTime *datetime_obj = (PyDateTime_DateTime *)obj;
    int offset = 0;

    // TODO: Support all kinds of tzinfo classes
    if (datetime_obj->hastzinfo) {
        if (Py_TYPE(datetime_obj->tzinfo) == &FixedOffset_type) {
            FixedOffset *tzinfo = (FixedOffset *)datetime_obj->tzinfo;
            offset = tzinfo->offset;
        }
    }

    date_time_struct dt = {
        {
            (datetime_obj->data[0] << 8) | datetime_obj->data[1],
            datetime_obj->data[2],
            datetime_obj->data[3],
            0, // wday, not needed
            1
        },
        {
            datetime_obj->data[4],
            datetime_obj->data[5],
            datetime_obj->data[6],
            (
                (datetime_obj->data[7] << 16) |\
                (datetime_obj->data[8] << 8) |\
                datetime_obj->data[9]
            ),
            offset,
            1
        },
        1
    };

    char datetime_string[33] = {0};
    _format_date_time(&dt, datetime_string);

    return PyString_FromString(datetime_string);
}

static PyObject *utcnow_to_string(PyObject *self) {
    date_time_struct dt = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, 0};
    _utcnow(&dt);

    char datetime_string[33] = {0};
    _format_date_time(&dt, datetime_string);

    return PyString_FromString(datetime_string);
}

static PyObject *localnow_to_string(PyObject *self) {
    date_time_struct dt = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, 0};
    _localnow(&dt);

    char datetime_string[33] = {0};
    _format_date_time(&dt, datetime_string);

    return PyString_FromString(datetime_string);
}

static PyObject *bench_c(PyObject *self) {
    return Py_None;
}

static PyMethodDef rfc3339_methods[] = {
    {
        "utcnow",
        (PyCFunction) utcnow,
        METH_NOARGS,
        "datetime aware object in UTC with current date and time."
    },
    {
        "now",
        (PyCFunction) localnow,
        METH_NOARGS,
        "datetime aware object in local timezone with current date and time."
    },
    {
        "from_rfc3339_string",
        (PyCFunction) from_rfc3339_string,
        METH_VARARGS,
        "Parse RFC3339 compliant date-time string."
    },
    {
        "to_rfc3339_string",
        (PyCFunction) to_rfc3339_string,
        METH_VARARGS,
        "Serialize datetime to RFC3339 compliant date-time string."
    },
    {
        "utcnow_to_string",
        (PyCFunction) utcnow_to_string,
        METH_NOARGS,
        "Current UTC date and time RFC3339 compliant date-time string."
    },
    {
        "now_to_string",
        (PyCFunction) localnow_to_string,
        METH_NOARGS,
        "Local date and time RFC3339 compliant date-time string."
    },
    {
        "bench_c",
        (PyCFunction) bench_c,
        METH_NOARGS,
        "Benchmark C code."
    },
    {NULL}
};

PyMODINIT_FUNC initrfc3339(void) {
    local_utc_offset = _get_local_utc_offset();

    PyObject *m;
    PyObject *version_string;

    PyDateTime_IMPORT;

    m = Py_InitModule("rfc3339", rfc3339_methods);
    if (m == NULL)
        return;

    version_string = PyString_FromString (RFC3339_VERSION);
    PyModule_AddObject(m, "__version__", version_string);

    FixedOffset_type.tp_new = PyType_GenericNew;
    FixedOffset_type.tp_base = PyDateTimeAPI->TZInfoType;
    FixedOffset_type.tp_methods = FixedOffset_methods;
    FixedOffset_type.tp_members = FixedOffset_members;
    FixedOffset_type.tp_init = (initproc)FixedOffset_init;

    if (PyType_Ready(&FixedOffset_type) < 0)
        return;

    Py_INCREF(&FixedOffset_type);
    PyModule_AddObject(m, "TZFixedOffset", (PyObject *)&FixedOffset_type);
}
