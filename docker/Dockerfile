# syntax=docker/dockerfile:1.7

FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCPP_BACKEND_CORE_BUILD_TESTS=OFF \
    && cmake --build build --config Release -j \
    && cmake --install build --prefix /opt/cpp-backend-core

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /srv/app
COPY --from=build /opt/cpp-backend-core /opt/cpp-backend-core
COPY config/app.json /srv/app/config/app.json

ENV APP_CONFIG=/srv/app/config/app.json
EXPOSE 8080
ENTRYPOINT ["/opt/cpp-backend-core/bin/cpp_backend_core", "/srv/app/config/app.json"]
