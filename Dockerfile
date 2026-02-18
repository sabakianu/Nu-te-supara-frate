FROM gcc:latest

WORKDIR /usr/src/game

COPY tabla.c .
COPY jucator.c .

RUN gcc tabla.c -o server -pthread

RUN gcc jucator.c -o client

EXPOSE 2729

CMD ["./server"]