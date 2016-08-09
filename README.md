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

|          | Support            | Performance optimized | Implementation |
| -------- |:------------------:|:---------------------:| -------------- |
| Python 2 | :heavy_check_mark: |  :heavy_check_mark:   | C              |
| Python 3 | :heavy_check_mark: |  :heavy_check_mark:   | C              |
| PyPy     | :heavy_check_mark: |  :heavy_check_mark:   | Pure Python    |

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

**Python 2**

```
$ sudo apt-get install python-dev build-essential
```

or

```
$ sudo yum install python-devel gcc
```

**Python 3**

```
$ sudo apt-get install python3-dev build-essential
```

or

```
$ sudo yum install python3-devel gcc
```

## Benchmark

The benchmarks compare the performance of equivalent code of `datetime` and
`udatetime`. The benchmark measures the time needed for 1 million executions
and takes the `min` of 3 repeats. You can run the benchmark yourself and see
the results on your machine by executing the `bench_udatetime.py` script.

![Benchmark interpreter summary](/extras/benchmark_interpreter_summary.png?raw=true "datetime vs. udatetime summary")

### Python 2.7

```
$ python scripts/bench_udatetime.py
Executing benchmarks ...

============ benchmark_parse
datetime_strptime 16.5012259483
udatetime_parse 1.38354206085
udatetime is 91% faster

============ benchmark_format
datetime_strftime 3.14105510712
udatetime_format 0.728137969971
udatetime is 76% faster

============ benchmark_utcnow
datetime_utcnow 0.537242174149
udatetime_utcnow 0.229556798935
udatetime is 57% faster

============ benchmark_now
datetime_now 1.72571802139
udatetime_now 0.248335123062
udatetime is 85% faster

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 3.87453198433
udatetime_utcnow_to_string 0.747572183609
udatetime is 80% faster

============ benchmark_now_to_string
datetime_now_to_string 6.03785800934
udatetime_now_to_string 0.76801109314
udatetime is 87% faster

============ benchmark_fromtimestamp
datetime_fromtimestamp 1.86422181129
udatetime_fromtimestamp 0.290146112442
udatetime is 84% faster

============ benchmark_utcfromtimestamp
datetime_utcfromtimestamp 0.613043069839
udatetime_utcfromtimestamp 0.278568983078
udatetime is 54% faster
```

### Python 3.5

```
$ python scripts/bench_udatetime.py
Executing benchmarks ...

============ benchmark_parse
datetime_strptime 16.13257666499976
udatetime_parse 1.5544983579998188
udatetime is 90% faster

============ benchmark_format
datetime_strftime 4.891585199000019
udatetime_format 0.794836056999884
udatetime is 83% faster

============ benchmark_utcnow
datetime_utcnow 0.7425550630000544
udatetime_utcnow 0.24937228799990407
udatetime is 66% faster

============ benchmark_now
datetime_now 1.9882010840001385
udatetime_now 0.27323114099999657
udatetime is 86% faster

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 6.06995576700001
udatetime_utcnow_to_string 0.8431572290000986
udatetime is 86% faster

============ benchmark_now_to_string
datetime_now_to_string 8.150880190999942
udatetime_now_to_string 0.8323142369999914
udatetime is 89% faster

============ benchmark_fromtimestamp
datetime_fromtimestamp 1.8858458029999383
udatetime_fromtimestamp 0.320553235999796
udatetime is 83% faster

============ benchmark_utcfromtimestamp
datetime_utcfromtimestamp 0.7209223129998463
udatetime_utcfromtimestamp 0.3411805099999583
udatetime is 52% faster
```

### PyPy 5.3.1

```
$ python scripts/bench_udatetime.py
Executing benchmarks ...

============ benchmark_parse
datetime_strptime 3.06690311432
udatetime_parse 0.921170949936
udatetime is 69% faster

============ benchmark_format
datetime_strftime 1.04332399368
udatetime_format 0.178369998932
udatetime is 82% faster

============ benchmark_utcnow
datetime_utcnow 0.174169063568
udatetime_utcnow 0.159958839417
udatetime is 8% faster

============ benchmark_now
datetime_now 0.953182935715
udatetime_now 0.168534994125
udatetime is 82% faster

============ benchmark_utcnow_to_string
datetime_utcnow_to_string 1.49676418304
udatetime_utcnow_to_string 0.748476028442
udatetime is 49% faster

============ benchmark_now_to_string
datetime_now_to_string 2.8361260891
udatetime_now_to_string 0.64388012886
udatetime is 77% faster

============ benchmark_fromtimestamp
datetime_fromtimestamp 0.887798070908
udatetime_fromtimestamp 0.10493516922
udatetime is 88% faster

============ benchmark_utcfromtimestamp
datetime_utcfromtimestamp 0.104001045227
udatetime_utcfromtimestamp 0.1032371521
udatetime is 0% faster
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
