from setuptools import setup, find_packages, Extension
import os
import sys

try:
    import __pypy__
except ImportError:
    __pypy__ = None

__version__ = None
here = os.path.abspath(os.path.dirname(__file__))
name = 'udatetime'

with open('%s/requirements.txt' % here) as f:
    requires = f.readlines()

with open('%s/version.txt' % here) as f:
    __version__ = f.readline().strip()

with open('%s/README.md' % here) as f:
    readme = f.readline().strip()

macros = []

if __pypy__ is not None:
    macros.append(('_PYPY', '1'))
elif sys.version_info.major == 2:
    macros.append(('_PYTHON2', '1'))
elif sys.version_info.major == 3:
    macros.append(('_PYTHON3', '1'))

rfc3339 = Extension(
    'rfc3339',
    ['./src/rfc3339.c'],
    define_macros=macros,
    extra_compile_args=['-Ofast', '-std=c99']
)

setup(
    name=name,
    version=__version__,
    description='Fast RFC3339 compliant date-time library',
    long_description=readme,
    license='Apache 2.0',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Operating System :: POSIX',
        'Operating System :: MacOS',
        'Programming Language :: C',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: Implementation :: CPython',
        'Programming Language :: Python :: Implementation :: PyPy',
    ],
    author='Simon Pirschel',
    author_email='simon@aboutsimon.com',
    url='https://github.com/freach/udatetime',
    packages=find_packages(),
    include_package_data=True,
    zip_safe=False,
    install_requires=requires,
    ext_modules=[rfc3339],
    scripts=['scripts/bench_udatetime.py'],
)
