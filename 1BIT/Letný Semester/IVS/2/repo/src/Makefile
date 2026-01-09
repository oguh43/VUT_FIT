### 
 # @module Makefile
 # @description Used to run/ build the app.
 # @author Hugo Bohácsek (xbohach00)
 # @author Filip Jenis (xjenisf00)
 ##

TEAM = xjenisf00_xbohach00_xdubnis00_xklcovl00
ARCHIVE_NAME = ${TEAM}.zip

.PHONY: doc run package make publish lint install test

all: install
	npm i -g jsdoc electron-forge pkg
	pkg stddev/index.js -t node16-linux-x64 --output stddev/stddev
	
# https://esdoc.org/ git shortlog --summary --numbered --all --no-merges
doc:
	cd .. && jsdoc -d docs --configure ./src/jsdocConf.json

run:
	cd .. && npm run start

package:
	cd .. && npm run make

make:
	cd .. && npm run make

publish:
	cd .. && npm run publish

lint:
	cd .. && npm run lint

install:
	cd .. && npm install

test:
	cd .. && npm run test

clean:
	rm -rf ../out/
	rm -rf ../node_modules/
	rm -rf ../docs/

profile:
	./stddev/stddev < test_input.txt

pack: doc make
	git clone ../ repo
	rm -f ${ARCHIVE_NAME}
	mkdir -p ${TEAM}
	cp -r repo docs ${TEAM}
	cp -a ../out/make/deb/x64/. ${TEAM}
	zip -r ${ARCHIVE_NAME} ${TEAM}
	rm -rf repo/ docs install ${TEAM}

help:
	$(info This app/Makefile targets require nodejs and npm to be installed in system)
