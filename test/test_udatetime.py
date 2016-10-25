import unittest
from datetime import datetime, timedelta, tzinfo
import udatetime


class Test(unittest.TestCase):

    def test_utcnow(self):
        dt_now = datetime.utcnow()
        now = udatetime.utcnow()

        self.assertIsInstance(now, datetime)
        self.assertEqual(now.year, dt_now.year)
        self.assertEqual(now.month, dt_now.month)
        self.assertEqual(now.day, dt_now.day)
        self.assertEqual(now.hour, dt_now.hour)
        self.assertEqual(now.minute, dt_now.minute)
        self.assertEqual(now.second, dt_now.second)
        # self.assertEqual(now.microsecond, dt_now.microsecond)

    def test_now(self):
        dt_now = datetime.now()
        now = udatetime.now()

        self.assertIsInstance(now, datetime)
        self.assertEqual(now.year, dt_now.year)
        self.assertEqual(now.month, dt_now.month)
        self.assertEqual(now.day, dt_now.day)
        self.assertEqual(now.hour, dt_now.hour)
        self.assertEqual(now.minute, dt_now.minute)
        self.assertEqual(now.second, dt_now.second)
        # self.assertEqual(now.microsecond, dt_now.microsecond)

    def test_from_and_to_string(self):
        rfc3339 = '2016-07-15T12:33:20.123000+01:30'
        dt = udatetime.from_string(rfc3339)

        self.assertIsInstance(dt, datetime)
        self.assertEqual(dt.year, 2016)
        self.assertEqual(dt.month, 7)
        self.assertEqual(dt.day, 15)
        self.assertEqual(dt.hour, 12)
        self.assertEqual(dt.minute, 33)
        self.assertEqual(dt.second, 20)
        self.assertEqual(dt.microsecond, 123000)
        self.assertEqual(udatetime.to_string(dt), rfc3339)

        rfc3339 = '2016-07-18T12:58:26.485897-02:00'
        dt = udatetime.from_string(rfc3339)
        self.assertEqual(udatetime.to_string(dt), rfc3339)

    def test_fromtimestamp(self):
        DAY = 86400
        HOUR = 3600
        TZ_CEST = udatetime.TZFixedOffset(60 * 2)

        for t in range(0, DAY - (2 * HOUR), HOUR):
            dt = datetime.fromtimestamp(t)
            udt = udatetime.fromtimestamp(t)

            self.assertIsInstance(udt, datetime)
            self.assertEqual(udt.year, dt.year)
            self.assertEqual(udt.month, dt.month)
            self.assertEqual(udt.day, dt.day)

            self.assertEqual(udt.hour, dt.hour)
            self.assertEqual(udt.minute, dt.minute)
            self.assertEqual(udt.second, dt.second)
            self.assertEqual(udt.microsecond, dt.microsecond)

        for t in range(0, DAY, HOUR):
            dt = datetime.fromtimestamp(t, TZ_CEST)
            udt = udatetime.fromtimestamp(t, TZ_CEST)

            self.assertIsInstance(udt, datetime)
            self.assertEqual(udt.year, dt.year)
            self.assertEqual(udt.month, dt.month)
            self.assertEqual(udt.day, dt.day)

            self.assertEqual(udt.hour, dt.hour)
            self.assertEqual(udt.minute, dt.minute)
            self.assertEqual(udt.second, dt.second)
            self.assertEqual(udt.microsecond, dt.microsecond)

        for t in range(0, DAY * -1, HOUR * -1):
            dt = datetime.fromtimestamp(t, TZ_CEST)
            udt = udatetime.fromtimestamp(t, TZ_CEST)

            self.assertIsInstance(udt, datetime)
            self.assertEqual(udt.year, dt.year)
            self.assertEqual(udt.month, dt.month)
            self.assertEqual(udt.day, dt.day)

            self.assertEqual(udt.hour, dt.hour)
            self.assertEqual(udt.minute, dt.minute)
            self.assertEqual(udt.second, dt.second)
            self.assertEqual(udt.microsecond, dt.microsecond)

    def test_utcfromtimestamp(self):
        DAY = 86400
        HOUR = 3600

        for t in range(0, DAY, HOUR):
            dt = datetime.utcfromtimestamp(t)
            udt = udatetime.utcfromtimestamp(t)

            self.assertIsInstance(udt, datetime)
            self.assertEqual(udt.year, dt.year)
            self.assertEqual(udt.month, dt.month)
            self.assertEqual(udt.day, dt.day)
            self.assertEqual(udt.hour, dt.hour)
            self.assertEqual(udt.minute, dt.minute)
            self.assertEqual(udt.second, dt.second)
            self.assertEqual(udt.microsecond, dt.microsecond)

        for t in range(0, DAY * -1, HOUR * -1):
            dt = datetime.utcfromtimestamp(t)
            udt = udatetime.utcfromtimestamp(t)

            self.assertIsInstance(udt, datetime)
            self.assertEqual(udt.year, dt.year)
            self.assertEqual(udt.month, dt.month)
            self.assertEqual(udt.day, dt.day)
            self.assertEqual(udt.hour, dt.hour)
            self.assertEqual(udt.minute, dt.minute)
            self.assertEqual(udt.second, dt.second)
            self.assertEqual(udt.microsecond, dt.microsecond)

    def test_broken_from_string(self):
        invalid = [
            '2016-07-15 12:33:20.123000+01:30',
            '2016-13-15T12:33:20.123000+01:30',
            '20161315T12:33:20.123000+01:30',
            '2016-07-15T12:33:20.1 +01:30',
            'Hello World',
            '2016-07-15 12:33:20.123000+01:302016-07-15 12:33:20.123000+01:30'
        ]

        for r in invalid:
            with self.assertRaises(ValueError):
                udatetime.from_string(r)

    def test_ok_from_string(self):
        rfc3339s = [
            '2016-07-15 T 12:33:20.123000 +01:30',
            '2016-07-15 T 12:33:20.123000 +01:30',
            '2016-07-15T12:33:20.123 +01:30',
            '2016-07-15T12:33:20 +01:30',
            '2016-07-15T12:33:20 Z',
            '2016-07-15T12:33:20',
            '2016-07-15t12:33:20'
        ]

        for r in rfc3339s:
            self.assertIsInstance(
                udatetime.from_string(r),
                datetime
            )

    def test_tzone(self):
        rfc3339 = '2016-07-15T12:33:20.123000+01:30'
        dt = udatetime.from_string(rfc3339)
        offset = dt.tzinfo.utcoffset()

        self.assertIsInstance(offset, timedelta)
        self.assertEqual(offset.total_seconds() / 60, 90)

        rfc3339 = '2016-07-15T12:33:20.123000Z'
        dt = udatetime.from_string(rfc3339)
        offset = dt.tzinfo.utcoffset()

        self.assertIsInstance(offset, timedelta)
        self.assertEqual(offset.total_seconds(), 0)

        rfc3339 = '2016-07-15T12:33:20.123000-02:00'
        dt = udatetime.from_string(rfc3339)
        offset = dt.tzinfo.utcoffset()

        self.assertIsInstance(offset, timedelta)
        self.assertEqual(offset.total_seconds() / 60, -120)

    def test_precision(self):
        t = 1469897308.549871
        dt = datetime.fromtimestamp(t)
        udt = udatetime.fromtimestamp(t)
        self.assertEqual(udt.microsecond, dt.microsecond)

    def test_raise_on_not_TZFixedOffset(self):
        class TZInvalid(tzinfo):
            def utcoffset(self, dt=None):
                return timedelta(seconds=0)

            def dst(self, dt=None):
                return timedelta(seconds=0)

        dt = datetime.now(TZInvalid())

        with self.assertRaises(ValueError):
            udatetime.to_string(dt)

if __name__ == '__main__':
    unittest.main()
