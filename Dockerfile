FROM gcc:latest


RUN apt-get update && apt-get install -y make \
    python3 \
    python3-serial

WORKDIR /app

COPY . .

RUN make clean
RUN make

EXPOSE 9000
EXPOSE 9001


RUN chmod +x /app/entrypoint.sh
CMD ["/app/entrypoint.sh"]