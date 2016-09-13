FROM ubuntu:16.04
MAINTAINER Simon Pirschel <simon@aboutsimon.com>
RUN apt-get update && apt-get install -y python python-dev python-pip python-setuptools build-essential git
RUN pip install git+https://github.com/freach/udatetime.git
