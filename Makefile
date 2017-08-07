MAINTAINER="Simon Pirschel <simon@aboutsimon.com>"

all: clean package

clean:
	rm -f *.deb
	rm -fr build
	rm -fr *.egg-info
	rm -fr dist
	find . -name '*.pyc' -delete
	find . -name '*.so' -delete
	rm -fr dist_eggs

package: clean
	python setup.py bdist sdist
	python setup.py bdist_egg

release: package
        pip install twine
        twine upload dist/*

.PHONY: clean package release
