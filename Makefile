# File              : Makefile
# Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
# Date              : 11.10.2021
# Last Modified Date: 30.10.2021
# Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
#

all:
	mkdir -p build
	cd build && cmake .. && make && ./yclients_test 

clean:
	rm -fr build

