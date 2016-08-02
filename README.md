# udatetime: Fast RFC3339 compliant date-time library

Handling date-times is a painful act because of the sheer endless amount
of formats used by people. Luckily there are a couple of specified standards
out there like [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601). But even
ISO 8601 leaves to many options on how to define date and time. That's why I
encourage using the [RFC3339](https://www.ietf.org/rfc/rfc3339.txt) specified
date-time format.

`udatetime` offers on average 76% faster `datetime` object instantiation,
serialization and deserialization of RFC3339 date-time strings. `udatetime` is
using Python's [datetime class](https://docs.python.org/2/library/datetime.html)
under the hood and code already using `datetime` should be able to easily
switch to `udatetime`. All `datetime` objects created by `udatetime` are timezone
aware.

**Python 2**

- Support: :heavy_check_mark:
- Performance optimized: :heavy_check_mark:
- Implementation: C

**Python 3**

- Support: :heavy_check_mark:
- Performance optimized: :x: (only RFC3339 parser)
- Implementation: Pure Python

**PyPy**

- Support: :heavy_check_mark:
- Performance optimized: :x: (only RFC3339 parser)
- Implementation: Pure Python


```python
>>> udatetime.from_string("2016-07-15T12:33:20.123000+02:00")
datetime.datetime(2016, 7, 15, 12, 33, 20, 123000, tzinfo=+02:00)

>>> dt = udatetime.from_string("2016-07-15T12:33:20.123000+02:00")
>>> udatetime.to_string(dt)
'2016-07-15T12:33:20.123000+02:00'

>>> udatetime.now()
datetime.datetime(2016, 7, 29, 10, 15, 24, 472467, tzinfo=+02:00)

>>> udatetime.utcnow()
datetime.datetime(2016, 7, 29, 8, 15, 36, 184762, tzinfo=+00:00)

>>> udatetime.now_to_string()
'2016-07-29T10:15:46.641233+02:00'

>>> udatetime.utcnow_to_string()
'2016-07-29T08:15:56.129798+00:00'

>>> udatetime.to_string(udatetime.utcnow() - timedelta(hours=6))
'2016-07-29T02:16:05.770358+00:00'

>>> udatetime.fromtimestamp(time.time())
datetime.datetime(2016, 7, 30, 17, 45, 1, 536586, tzinfo=+02:00)

>>> udatetime.utcfromtimestamp(time.time())
datetime.datetime(2016, 8, 1, 10, 14, 53, tzinfo=+00:00)
```

## Installation

Currently only **POSIX** compliant systems are supported.
Working on cross-platform support.

```
$ pip install udatetime
```

You might need to install the header files of your Python installation and
some essential tools to execute the build like a C compiler.

```
$ sudo apt-get install python-dev build-essential
```

or

```
$ sudo yum install python-devel gcc
```

## Benchmark

The benchmarks compare the performance of equivalent code of `datetime` and
`udatetime`. The benchmark measures the time needed for 1 million executions
and takes the `min` of 3 repeats. You can run the benchmark yourself and see
the results on your machine by executing the `bench.py` script.

### Python 2.7

```
$ python bench.py
Executing benchmarks ...

============ benchmark_parse
datetime_strptime 11.1304590702
udatetime_parse 1.15330886841
udatetime is 89% faster

============ benchmark_format
datetime_strftime 2.11881709099
udatetime_format 0.681449890137
udatetime is 67% faster

============ benchmark_utcnow
datetime_utcnow 0.510785102844
udatetime_utcnow 0.250232934952
udatetime is 51% faster

============ benchmark_now
datetime_now 1.47083210945
udatetime_now 0.251370191574
udatetime is 82% faster

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 2.7153339386
udatetime_utcnow_to_string 0.744527816772
udatetime is 72% faster

============ benchmark_now_to_string
datetime_now_to_string 3.9927649498
udatetime_now_to_string 0.737413883209
udatetime is 81% faster

============ benchmark_fromtimestamp
datetime_fromtimestamp 1.48366808891
udatetime_fromtimestamp 0.298361063004
udatetime is 79% faster

============ benchmark_utcfromtimestamp
datetime_utcfromtimestamp 0.551414012909
udatetime_utcfromtimestamp 0.2882771492
udatetime is 47% faster
```

### Python 3.5

```
$ python bench.py
Executing benchmarks ...

============ benchmark_parse
datetime_strptime 10.420707688999755
udatetime_parse 5.969888809000622
udatetime is 42% faster

============ benchmark_format
datetime_strftime 3.093145213999378
udatetime_format 3.409761550999974
udatetime is 9% slower

============ benchmark_utcnow
datetime_utcnow 0.5723336070004734
udatetime_utcnow 2.4757570670008135
udatetime is 76% slower

============ benchmark_now
datetime_now 1.5356289419996756
udatetime_now 3.681305574000362
udatetime is 58% slower

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 3.6737609150004573
udatetime_utcnow_to_string 8.396922039000856
udatetime is 56% slower

============ benchmark_now_to_string
datetime_now_to_string 5.066462685999795
udatetime_now_to_string 11.217101648999233
udatetime is 54% slower

============ benchmark_fromtimestamp
datetime_fromtimestamp 1.49455352699988
udatetime_fromtimestamp 1.7328117040015059
udatetime is 13% slower

============ benchmark_utcfromtimestamp
datetime_utcfromtimestamp 0.6809496840014617
udatetime_utcfromtimestamp 0.8325351729999966
udatetime is 18% slower
```

### PyPy 5.1.1

```
$ python bench.py
Executing benchmarks ...

============ benchmark_parse
datetime_strptime 2.63264703751
udatetime_parse 0.895127058029
udatetime is 65% faster

============ benchmark_format
datetime_strftime 1.11581993103
udatetime_format 1.13203406334
udatetime is 1% slower

============ benchmark_utcnow
datetime_utcnow 0.195902109146
udatetime_utcnow 0.355302095413
udatetime is 44% slower

============ benchmark_now
datetime_now 1.14692687988
udatetime_now 1.33081102371
udatetime is 13% slower

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 1.50462412834
udatetime_utcnow_to_string 1.79078006744
udatetime is 15% slower

============ benchmark_now_to_string
datetime_now_to_string 2.50896692276
udatetime_now_to_string 2.9683971405
udatetime is 15% slower

============ benchmark_fromtimestamp
datetime_fromtimestamp 0.941156864166
udatetime_fromtimestamp 0.943603992462
udatetime is 0% slower

============ benchmark_utcfromtimestamp
datetime_utcfromtimestamp 0.103535890579
udatetime_utcfromtimestamp 0.107717037201
udatetime is 3% slower
```

## Why RFC3339

The RFC3339 specification has the following advantages:

- Defined date, time, timezone, date-time format
- 4 digit year
- Fractional seconds
- Human readable
- No redundant information like weekday name
- Simple specification, easily machine readable

### RFC3339 format specification
```
date-fullyear   = 4DIGIT
date-month      = 2DIGIT  ; 01-12
date-mday       = 2DIGIT  ; 01-28, 01-29, 01-30, 01-31 based on
                          ; month/year
time-hour       = 2DIGIT  ; 00-23
time-minute     = 2DIGIT  ; 00-59
time-second     = 2DIGIT  ; 00-58, 00-59, 00-60 based on leap second
                          ; rules
time-secfrac    = "." 1*DIGIT
time-numoffset  = ("+" / "-") time-hour ":" time-minute
time-offset     = "Z" / time-numoffset

partial-time    = time-hour ":" time-minute ":" time-second [time-secfrac]

full-date       = date-fullyear "-" date-month "-" date-mday
full-time       = partial-time time-offset

date-time       = full-date "T" full-time
```

`udatetime` specific format addons:

- time-secfrac can be either 3DIGIT for milliseconds or 6DIGIT for microseconds
- time-secfrac will be normalized to microseconds
- time-offset is optional. Missing time-offset will be treated as UTC.
- spaces will be eliminated

## Why in C?

The Python `datetime` library is flexible but painfully slow, when it comes to
parsing and formating. High performance applications like web services or
logging benefit from fast underlying libraries. Restricting the input format
to one standard allows for optimization and results in speed improvements.
