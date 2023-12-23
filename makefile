build: semaphore.cpp
	g++ -o sem semaphore.cpp -pthread -lrt
clean:
	rm -f sem

