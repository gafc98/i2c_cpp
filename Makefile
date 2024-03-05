i2c_test: i2c_test.cpp
	g++ -fdiagnostics-color=always -g i2c_test.cpp -Ofast -fconcepts-ts -std=c++17 -o i2c_test

clean:
	rm i2c_test