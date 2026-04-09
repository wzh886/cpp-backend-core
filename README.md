# cpp-backend-core

![CI](https://github.com/wzh886/cpp-backend-core/actions/workflows/ci.yml/badge.svg)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Release](https://img.shields.io/github/v/release/wzh886/cpp-backend-core)

A production-minded C++20 HTTP backend starter designed to look credible in a real portfolio: modern CMake, clean modular boundaries, JSON configuration, structured logging, explicit error handling, health checks, example API routes, tests, and container packaging.

## Why this repo exists

Many C++ backend examples are either toy-level or framework-heavy. This starter targets the middle ground:

- small enough to understand in one sitting
- opinionated enough to ship as a serious foundation
- modular enough to extend into a real service
- polished enough to show recruiters or clients

## Features

- **C++20** with modern CMake
- **HTTP API** built on a lightweight POSIX socket server
- **Structured JSON logging** via an internal zero-dependency logger
- **Typed configuration model** with file loading and environment overrides
- **Consistent error envelope** for API failures
- **Health endpoint** for probes and load balancers
- **Example JSON endpoint** for request/response patterns
- **Minimal unit-test scaffold** registered with CTest
- **Dockerfile** for local demo or deployment bootstrap

## Project layout

```text
.
├── CMakeLists.txt
├── config/
│   └── app.json
├── docker/
│   └── Dockerfile
├── include/
│   ├── app/
│   ├── config/
│   ├── core/
│   ├── http/
│   └── logging/
├── src/
│   ├── app/
│   ├── config/
│   ├── core/
│   ├── http/
│   └── logging/
└── tests/
```

## Architecture

### Layers

1. **app**
   - bootstraps configuration, logging, and server startup
2. **config**
   - owns typed runtime configuration and env overrides
3. **core**
   - shared primitives such as application errors
4. **http**
   - request context, route registration, and response shaping
5. **logging**
   - structured JSON logger wrapper around standard output

### Request flow

```text
incoming request
  -> RequestContext builds request metadata
  -> route handler validates/parses payload
  -> business logic returns JSON
  -> failures map to AppError or internal error envelope
  -> JSON response + structured log entry emitted
```

## Quickstart

### Prerequisites

- CMake 3.24+
- C++20 compiler (`g++` 11+ or `clang++` 14+)
- POSIX-compatible OS (Linux/macOS; current implementation uses BSD/POSIX sockets)
- Git (optional)

### Build

```bash
cmake -S . -B build
cmake --build build -j
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

### Run locally

```bash
./build/cpp_backend_core config/app.json
```

The server listens on `0.0.0.0:8080` by default.

## Configuration

Default configuration lives at `config/app.json`.

### File example

```json
{
  "service_name": "cpp-backend-core",
  "environment": "development",
  "version": "0.1.0",
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "read_timeout_seconds": 5,
    "write_timeout_seconds": 5,
    "idle_interval_microseconds": 100000
  },
  "logging": {
    "level": "info"
  }
}
```

### Environment overrides

| Variable | Meaning |
|---|---|
| `APP_CONFIG` | path to config file |
| `APP_SERVICE_NAME` | service name override |
| `APP_ENV` | environment override |
| `APP_VERSION` | version override |
| `APP_HOST` | bind host override |
| `APP_PORT` | bind port override |
| `APP_LOG_LEVEL` | logger level override |

Example:

```bash
APP_PORT=9090 APP_LOG_LEVEL=debug ./build/cpp_backend_core config/app.json
```

## API

### `GET /healthz`

Used for uptime probes and orchestration checks.

**Example response**

```json
{
  "status": "ok",
  "service": "cpp-backend-core",
  "environment": "development",
  "version": "0.1.0",
  "request_id": "db0a4f2a4bc5d901"
}
```

### `GET /api/v1/info`

Returns service metadata and request context.

```bash
curl -s http://127.0.0.1:8080/api/v1/info | jq
```

### `POST /api/v1/echo`

Demonstrates JSON request parsing and response shaping.

```bash
curl -s \
  -X POST http://127.0.0.1:8080/api/v1/echo \
  -H 'Content-Type: application/json' \
  -d '{"message":"hello","tags":["portfolio","cpp"]}' | jq
```

**Example response**

```json
{
  "data": {
    "message": "hello",
    "tags": ["portfolio", "cpp"]
  },
  "meta": {
    "request_id": "2ff88c302a3173a5",
    "echoed_at": 1775738512000
  }
}
```

### Error shape

```json
{
  "error": {
    "code": "http.invalid_json",
    "message": "[json.exception.parse_error.101] parse error at line 1, column 2",
    "details": {},
    "request_id": "1d7c7a8e7f88e0a4"
  }
}
```

## Structured logging

Logs are emitted as one JSON object per line, suitable for ingestion by Fluent Bit, Vector, Loki, ELK, or cloud log pipelines.

Example log:

```json
{
  "client_ip": "127.0.0.1",
  "event": "request.completed",
  "level": "info",
  "method": "GET",
  "path": "/healthz",
  "request_id": "eb5fb6cf63f671a2",
  "route": "/healthz",
  "service": "cpp-backend-core",
  "status": 200,
  "timestamp_epoch_ms": 1775738512000
}
```

## Docker

Build and run:

```bash
docker build -f docker/Dockerfile -t cpp-backend-core .
docker run --rm -p 8080:8080 cpp-backend-core
```

## Design choices

- **Zero external runtime dependencies**: keeps the starter buildable in constrained environments and easy to audit.
- **JSON config + env overrides**: simple local development, container-friendly deployment.
- **Structured logs**: operationally useful from day one.
- **Explicit `AppError` model**: cleaner than throwing raw strings or leaking internal exceptions.
- **Modular directory layout**: supports growth into domains, persistence, auth, and background jobs.

## Roadmap

- Add graceful shutdown and signal handling
- Add dependency injection container or composition root
- Add middleware/auth abstraction
- Add Prometheus metrics endpoint
- Add OpenAPI generation or contract tests
- Add database adapter boundary with repository interfaces
- Add request tracing propagation (`x-request-id`, OpenTelemetry)
- Add CI with sanitizer and coverage jobs

## Recruiter/client value

This repo is intentionally framed as a serious backend starter rather than a coding exercise. It demonstrates:

- clean systems decomposition
- production-facing operational concerns
- modern build tooling
- testability and containerization
- readable C++ code without overengineering

## License

MIT or Apache-2.0 are sensible defaults if you plan to publish this publicly.
