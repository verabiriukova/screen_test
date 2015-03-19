LIBS    = -lbcm2835 -lrt

all: screen_test

ad7705_test: screen_test.o gz_clk.o gpio-sysfs.o
	gcc -o screen_test gpio-sysfs.o screen_test.o gz_clk.o $(LIBS)

ad7705_test.o: screen_test.cpp
	gcc -c screen_test.cpp

gz_clk.o: gz_clk.cpp gz_clk.h
	gcc -c gz_clk.cpp

gpio-sysfs.o: gpio-sysfs.cpp gpio-sysfs.h
	gcc -c gpio-sysfs.cpp

clean:
	rm -f screen_test screen_test.o gz_clk.o *~ gpio-sysfs.o
