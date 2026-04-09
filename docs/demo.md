# Demo Walkthrough

This document shows the expected local flow for `cpp-backend-core`.

## Build

```bash
cmake -S . -B build
cmake --build build -j4
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Expected result:

```text
100% tests passed, 0 tests failed out of 3
```

## Run

```bash
./build/cpp_backend_core config/app.json
```

## Sample requests

### Health

```bash
curl -s http://127.0.0.1:8080/healthz
```

Example response:

```json
{"status":"ok","service":"cpp-backend-core","environment":"development","version":"0.1.0","request_id":"90661dd27d949d27"}
```

### Info

```bash
curl -s http://127.0.0.1:8080/api/v1/info
```

Example response:

```json
{"service":{"name":"cpp-backend-core","environment":"development","version":"0.1.0"},"request":{"id":"b06bd7bd1d7dae82","method":"GET","path":"/api/v1/info","client_ip":"127.0.0.1"},"capabilities":["health","structured_logging","json_api","config_overrides"]}
```

### Echo

```bash
curl -s -X POST http://127.0.0.1:8080/api/v1/echo -H 'content-type: application/json' -d '{"hello":"world"}'
```

Example response:

```json
{"data":{"hello":"world"},"meta":{"request_id":"b8076d64e5e456de","echoed_at":"1775742781999"}}
```

## Why this matters

The goal of this repo is not just to compile. It should show a believable backend engineering baseline:
- structured runtime config
- repeatable build/test flow
- observable HTTP behavior
- small but coherent API surface
