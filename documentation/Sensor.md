# Generic Sensor Template

The `Sensor<T>` class is a **generic base class** for sensor monitors in ESP8266 projects. It provides the logic for:
* Periodic reading of sensor metrics
* Logging
* Broadcasting over WebSocket
* Serving REST API endpoints
* Attaching hooks for reporting steps and metrics

Specific sensors (e.g., `AHT10`, `BMP280`) only need to implement **how to read the sensor** and optionally how to check the validity of the metrics.

---

## Template Parameter

* `T` — the **metrics structure** for a given sensor.
  The struct must implement:

  ```cpp
  void toJson(JsonDocument& doc) const;
  ```

  and may optionally implement:

  ```cpp
  bool isValid() const;
  ```

Example:

```cpp
struct aht10Metrics {
    String time;
    float temperature;
    float humidity;

    void toJson(JsonDocument& doc) const {
        doc["time"] = time;
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
    }

    bool isValid() const {
        return !isnan(temperature) && !isnan(humidity);
    }
};
```

---

## Constructor

```cpp
Sensor(String sensorName, HTTPServerManager& serverManager, Logger* logger);
```

* **sensorName** — a unique name for the sensor (used in REST API endpoints).
* **serverManager** — reference to the `HTTPServerManager` (used for registering endpoints and broadcasting).
* **logger** — pointer to the `Logger` instance (may be `nullptr`).

---

## Public Methods

### `virtual void begin()`

Initializes the sensor and registers API endpoints.  Can be overridden or enriched in derived classes.

### `virtual void loop()`

Main loop function.
Periodically:

* Calls `read()` (implemented by derived class).
* Logs the metrics.
* Reports metrics to hooks.
* Broadcasts metrics to WebSocket clients.

The following diagram shows what happens inside `Sensor<T>::loop()` when the periodic timer expires:

```mermaid
sequenceDiagram
    participant Loop as Sensor<T>::loop()
    participant Sensor as Sensor<T>
    participant Derived as DerivedSensor (e.g. AHT10Monitor)
    participant Logger as Logger
    participant WS as WebSocket Clients
    participant Hooks as Report Hooks

    Loop->>Sensor: check if (millis - _lastTime > _periodicity)
    alt periodicity expired
        Loop->>Hooks: reportStep(1)

        Loop->>Derived: read()
        Derived-->>Sensor: metrics (struct T)

        Loop->>Logger: logMetrics(metrics)
        Loop->>Hooks: reportMetrics(metrics)
        Loop->>WS: broadcastMetrics(metrics)

        Loop->>Hooks: reportStep(2)
    else not yet
        Note over Loop: Do nothing
    end
```
This diagram shows the following order:

1. `loop()` checks timing.
2. If expired → `reportStep(1)` hook.
3. Calls `read()` from the derived class.
4. Logs, reports, broadcasts metrics.
5. Calls `reportStep(2)` hook.
6. Otherwise, does nothing.


The method can be overridden in derived classes.

### `virtual T read() = 0`

Pure virtual function.
Must be implemented by derived class to actually read sensor values.

### `T getLastMetrics() const`

Returns the last metrics that were read.

* Returns by value.
* Always safe, even if the original metrics object is modified later.

### `bool checkMetrics(const T& metric)`

Checks if metrics are valid.

* If `T` defines `isValid()`, that is called.
* Otherwise always returns `true`.

### `virtual void logMetrics(const T& metric)`

Logs metrics using the provided `Logger`.

* Default implementation serializes to JSON and logs it.
* Can be overridden in derived classes.

### `void broadcastMetrics(const T& metrics)`

Broadcasts the metrics as JSON over WebSocket.

### `void addReportStepHook(std::function<void(int)> func)`

Adds a callback that will be invoked when reporting steps (`loop()` start/end).

### `void reportStep(int step)`

Calls all registered step hooks.

### `void addReportMetricsHook(std::function<void(T&)> func)`

Adds a callback that will be invoked when metrics are reported.

### `void reportMetrics(T& metric)`

Calls all registered metrics hooks.

### `void registerEndpoints()`

Registers the following REST API endpoints:

* `GET /api/getLastMetrics<SENSOR_NAME>`
  → Returns the last metrics in JSON format.

* `GET /api/getPeriodicity<SENSOR_NAME>`
  → Returns the current periodicity in ms.

* `POST /api/setPeriodicity<SENSOR_NAME>`
  → Updates the periodicity. Body must be JSON:

  ```json
  { "periodicity": 5000 }
  ```

### `void setPeriodicity(int periodicity)`

Sets the read periodicity in milliseconds.

---

## Protected Members

* `HTTPServerManager& _serverManager`
* `Logger* _logger`
* `String _sensorName`
* `long _lastTime`
* `int _periodicity`
* `T _lastMetrics`
* Hook vectors:

  * `_reportStepsHooks`
  * `_reportMetricsHooks`

---

## Internal Method

### `String serializeMetrics(const T& metrics)`

Serializes metrics to JSON using `ArduinoJson`.
The output includes the sensor name and delegates serialization of fields to `metrics.toJson()`.

---

## Example: Creating a New Sensor
Given the struct `aht10Metrics` as in the example above, a class to monitor metrics from the AHT10 sensor can be implemented like so:
```cpp
class AHT10Monitor : public Sensor<aht10Metrics> {
public:
    AHT10Monitor(HTTPServerManager& serverManager, Logger* logger)
        : Sensor<aht10Metrics>("AHT10", serverManager, logger), 
          _AHT10(AHT10_ADDRESS_0X38) {}

    void begin() override {
        _AHT10.begin(SDA_PIN, SCL_PIN);
        Sensor::begin();
    }

    aht10Metrics read() override {
        aht10Metrics toret;
        toret.time = Logger::timeToString();

        toret.temperature = _AHT10.readTemperature();
        toret.humidity = _AHT10.readHumidity();
        return toret;
    }

private:
    AHT10 _AHT10;
};
```
