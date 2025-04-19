FROM gcc:latest


RUN apt-get update && apt-get install -y make


WORKDIR /app

COPY . .

RUN make clean
RUN make

CMD ["./bin/telemetrycore"]