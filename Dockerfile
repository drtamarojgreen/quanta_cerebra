FROM ubuntu:20.04
RUN apt-get update && apt-get install -y g++ cmake zip gunzip
COPY . /app
WORKDIR /app
RUN g++ -std=c++17 src/*.cpp -o QuantaCerebra -lpthread
ENTRYPOINT ["./QuantaCerebra"]
