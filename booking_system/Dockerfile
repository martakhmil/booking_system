FROM gcc:13

RUN apt update && apt install -y \
    libsqlite3-dev \
    libasio-dev \
    wget \
    nginx

WORKDIR /app

RUN wget https://github.com/CrowCpp/Crow/releases/download/v1.0+5/crow_all.h

COPY . .

RUN gcc -c sqlite3.c -o sqlite3.o && \
    g++ booking_system.cpp sqlite3.o \
    -pthread \
    -o /usr/local/bin/hotel

RUN printf 'server {\n\
    listen 8080;\n\
    location / {\n\
        if ($request_method = OPTIONS) {\n\
            add_header Access-Control-Allow-Origin *;\n\
            add_header Access-Control-Allow-Methods "GET, POST, PUT, DELETE, OPTIONS";\n\
            add_header Access-Control-Allow-Headers "Content-Type";\n\
            add_header Content-Length 0;\n\
            return 204;\n\
        }\n\
        add_header Access-Control-Allow-Origin * always;\n\
        add_header Access-Control-Allow-Methods "GET, POST, PUT, DELETE, OPTIONS" always;\n\
        add_header Access-Control-Allow-Headers "Content-Type" always;\n\
        proxy_pass http://127.0.0.1:18080;\n\
    }\n\
}\n' > /etc/nginx/sites-available/default

EXPOSE 8080

CMD service nginx start && /usr/local/bin/hotel