FROM gcc:13

RUN apt update && apt install -y \
libsqlite3-dev \
libasio-dev \
wget

WORKDIR /app

RUN wget https://github.com/CrowCpp/Crow/releases/download/v1.0+5/crow_all.h

COPY . .

RUN gcc -c sqlite3.c -o sqlite3.o && \
g++ booking_system.cpp sqlite3.o \
-pthread \
-o /usr/local/bin/hotel

EXPOSE 18080

CMD ["/usr/local/bin/hotel"]