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

## Benchmark

The benchmarks compare the performance of equivalent code of `datetime` and
`udatetime`. The benchmark measures the time needed for 1 million executions
and takes the `min` of 3 repeats. You can run the benchmark yourself and see
the results on your machine by executing the `bench.py` script.

```
$ python bench.py
Executing benchmarks ...

============ benchmark_format
datetime_strftime 2.47099590302
udatetime_format 0.729417085648
Difference: 70%

============ benchmark_now
datetime_now 1.43802499771
udatetime_now 0.206050157547
Difference: 85%

============ benchmark_parse
datetime_strptime 11.241353035
udatetime_parse 1.14998006821
Difference: 89%

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 2.64023208618
udatetime_utcnow_to_string 0.668451070786
Difference: 74%

============ benchmark_utcnow
datetime_utcnow 0.509386062622
udatetime_utcnow 0.209677934647
Difference: 58%

============ benchmark_now_to_string
datetime_now_to_string 3.82465291023
udatetime_now_to_string 0.66116809845
Difference: 82%
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
