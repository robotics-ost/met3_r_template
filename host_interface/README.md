# Host side ESP32 UDP telemetry/command interface

> [!TIP]
> **Just want to use the web GUI?** Follow the step-by-step guide [Web GUI](../docs/WEB_GUI.md). This README describes the details of the protocol and how to adapt the GUI.

## Overview

```
host_interface/
├── esp32_config.json.example   <- copy to esp32_config.json and edit (see "Config file" below)
├── core/
│   └── udp_core.py             <- single source of truth for the wire protocol,
│                                   struct packing/parsing, socket + receiver thread
└── gui_web/                    <- browser dashboard (Flask + Chart.js), imports core/
    ├── app.py
    ├── requirements.txt
    ├── templates/index.html
    └── static/{app.js, style.css}
```

`core/udp_core.py` has zero dependencies beyond the Python standard
library, so it's also usable directly for a plain command-line workflow
without the GUI (see "Command-line-only usage" below).

Only one process can be registered as the ESP32's telemetry destination
at a time, because the firmware only remembers whoever last sent it a
`HELLO` packet -- e.g. stop the web GUI before running a separate
command-line script against the same ESP32, or vice versa.

## Shared core (`core/udp_core.py`)

This file has zero dependencies beyond the Python
standard library, so it can also be used directly for quick
scripting/testing.

### Config file (`esp32_config.json`)

The UI accepts the ESP32's address as a command line argument, **or**
reads it from a small JSON config file so you don't have to retype it
every time. Just copy the example JSON config file and edit it:

```bash
cp esp32_config.json.example esp32_config.json
```
```json
{
  "esp32_host": "robot_1.local",
  "esp32_port": 3333
}
```

Resolution order (see `find_config_path()` in `udp_core.py`): an
explicit path argument, then the `ESP32_CONFIG_PATH` environment
variable, then `./esp32_config.json` in the current directory, then
`esp32_config.json` at the repo root. A CLI argument always overrides the
config file when both are given.

`esp32_host` can be a plain IP, a regular DNS hostname, or an mDNS
`.local` name -- `.local` resolution depends on your OS having mDNS set up
(built in on macOS; needs `avahi-daemon` + `libnss-mdns` on Linux;
Bonjour on Windows). If resolution fails you'll get a clear error message.

### Python Usage

```python
from udp_core import UdpClient

client = UdpClient("192.168.1.50")            # IP...
client = UdpClient("robot_1.local")       # ...or a hostname
client.on_telemetry(lambda batch: print(batch.seq, batch.control_samples))
client.start()          # resolves the host, sends HELLO, starts the receiver thread

client.send_move_to(1.0, 0.5, 0.0)
client.send_wait(2000)
client.send_remove_step(3)
client.send_safety(1)

client.stop()           # sends GOODBYE, stops the thread
```

### Command-line usage

If you just want a plain-terminal workflow without the GUI, it's a
~15-line script using the same core:

```python
from udp_core import UdpClient

client = UdpClient("192.168.1.50")
client.on_telemetry(lambda b: print(b))
client.start()
try:
    while True:
        cmd = input("> ")
        # ... parse cmd, call client.send_*() ...
finally:
    client.stop()
```

## Web GUI (`gui_web/`)

Local virtual environment (recommended):
```bash
cd gui_web
python3 -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate
pip install -r requirements.txt
python app.py <ESP32_HOST> [--port 3333] [--http-port 8080]
# or, with esp32_config.json in place, no args needed at all:
python app.py
```

Open `http://localhost:8080`. Telemetry streams to the browser over
Server-Sent Events (no websocket library needed); commands are sent via
plain `fetch()` POSTs to Flask routes, which call `UdpClient.send_*`.

**Runtime-configurable plots**: click "+ Add plot" to create a new panel,
click the signal chips to choose which telemetry values it shows, and
edit the window (seconds) field. The default panels are set up in `defaultPanels()` in
`static/app.js`; feel free to change what it creates by default, or add
a config-file-driven default layout later if that becomes worth it.

The sequencer queue is shown as a row of cards (id, type, and its
relevant fields) in the sidebar, next to the "remove step" form, so you
can see which `step_id` to remove.

**Requires internet access once** to load Chart.js from a CDN
(`cdn.jsdelivr.net`) in `templates/index.html`. If the GUI needs to run on
an offline network, download `chart.umd.min.js` and serve it from
`gui_web/static/` instead, then update the `<script src=...>` tag.

## Adapting the protocol

Three parts of the protocol are expected to change as the firmware
evolves, and each touches a specific, fixed set of files. **None of this
is automatic** -- `core/udp_core.py` owns the wire format, but the GUI's
field names, JSON keys, and display logic are hand-maintained and don't
derive from it, so every location below needs updating by hand. Miss one
and the most common symptom is *not* a crash or a visible error: the
browser tab just silently shows stale or no data (check the browser's
JavaScript console -- F12 -- for a `TypeError` if that happens).

### A. Changing what's in a control sample (periodic telemetry values)

This is the data the ESP32 streams continuously (e.g. sensor readings or global pose and velocity). To change which values it carries:

1. **`core/udp_core.py`**
   - `CONTROL_SAMPLE_FMT` -- the `struct` format string. Must match the
     firmware's packed C struct byte-for-byte (same fields, same order,
     same types).
   - `ControlSample` dataclass -- add/rename/remove fields to match.
   - `parse_telemetry()` -- the line that unpacks a control sample
     (`ts, ... = struct.unpack_from(CONTROL_SAMPLE_FMT, ...)`) and builds
     a `ControlSample(...)` from it. Field names/order must match the
     dataclass exactly.
2. **`gui_web/app.py`**, `_on_telemetry()` -- the dict comprehension that
   builds the `"control_samples"` list for the browser. Update the keys
   to match your new/renamed fields; this JSON shape is what the frontend
   actually consumes, and it's a separate, hand-written mapping, not
   generated from the dataclass.
3. **`gui_web/static/app.js`**
   - `SIGNALS` registry -- add/rename/remove an entry per signal. Each
     entry's key must exactly match a key you used in the JSON from step 2.
     `source: "control"` for anything from a control sample.
   - `connectStream()`'s ingestion loop -- the `pushSample("<key>", tSec,
     c.<key>)` calls, one per signal, must reference the same keys.
   - `defaultPanels()` -- reference your new signal keys here if you want
     them shown by default.
4. **`gui_web/static/style.css`** -- add a `--sig-<name>` custom property
   under `:root` for each new signal (used as its chart line color);
   remove ones no longer referenced.

### B. Adding a new sequencer step type

Currently there are two step types, `WAIT` (0) and `MOVE_TO` (1). In the
telemetry, every step travels in one fixed-size struct (`step_type, step_id,
duration_ms, x, y, phi`); fields that don't apply to a step's type are zero
(`WAIT` uses only `duration_ms`, `MOVE_TO` only `x, y, phi`).

To add a new step type (with its own command to queue it), change the
firmware **and** the PC side. Use `WAIT` and `MOVE_TO` as templates.

**ESP32 firmware**

1. **`lib/sequencer/include/sequencer.h`**
   - Add a value to `sequencer_step_type_t` (before `_COUNT`).
   - If the step has parameters: add a struct to the union in
     `sequencer_step_t`.
2. **`lib/sequencer/src/sequencer.c`**
   - `sequencer_step_type_to_string()` -- add a `case`.
   - `sequencer_task()` -- add a `case` to the `switch`. A step is executed
     in several cycles: initialize it in the first cycle, check in every
     following cycle whether it is completed, and handle an abort (the step
     was removed from the queue). Never wait inside the `case`.
3. **`lib/udp_interface/src/udp_interface.c`**
   - Add a `MSG_TYPE_CMD_*` constant and a packed struct for the command
     (same byte layout as on the PC side, see below).
   - `handle_command()` -- add a `case` that creates the step and adds it to
     the queue with `sequencer_add_step_to_queue()`.
   - If the web GUI should display the parameters of the step: add fields to
     the packed struct `telemetry_sequencer_sample_t` and fill them in the loop
     that fills `packet.sequencer_samples`. Set the fields that don't belong to
     the step type to zero.

**PC (host interface)**

1. **`core/udp_core.py`**
   - Add a new `STEP_TYPE_*` constant (same value as in the firmware).
   - Add a new `MSG_TYPE_CMD_*` constant (same value as in the firmware),
     a new `..._FMT` struct format, a new `build_..._cmd()` function, and a
     new `send_...()` method on `UdpClient` -- mirror `build_move_to_cmd()` /
     `send_move_to()`.
   - If the step has parameters that the web GUI should display:
     `SEQUENCER_STEP_FMT`, the `SequencerStep` dataclass and the
     step-unpacking lines in `parse_telemetry()`. They must match
     `telemetry_sequencer_sample_t` in the firmware. Every step type shares the
     same struct layout, so new fields must make sense as "zero when unused"
     for the existing types too.
2. **`gui_web/app.py`**
   - A new `/api/command/<name>` Flask route calling your new `send_...()`
     method -- mirror `cmd_move_to()`.
   - `_on_telemetry()`'s `"steps"` JSON list -- add the new telemetry fields.
3. **`gui_web/static/app.js`**
   - `renderSteps()` -- add a `case` for the new `step_type` (label and detail
     text of the card).
   - `wireCommandForms()` -- wire up the form of the new command.
4. **`gui_web/templates/index.html`** -- add a form for the new command,
   mirroring the **Move to** form.
5. **`gui_web/static/style.css`** -- add a `.step-card.type-<name>` rule
   if you want the new step type visually distinct, mirroring
   `.step-card.type-wait`.

> [!WARNING]
> The packed C struct of the command and the Python `struct` format must
> match **byte for byte** (order, types, size). If they don't, the firmware
> silently ignores the command: there is no error message.

### C. Changing safety events and safety levels

These are two independent, unrelated numeric codes: a safety **event**
is a command you send to the ESP32 (it may trigger a transition); a
safety **level** is a state the ESP32 reports back in telemetry. Neither
is currently validated or enumerated in `core/udp_core.py` -- both travel
as a plain `uint8`, so changing the *set* of values needs no core changes
at all, only GUI changes:

- **To change which safety events can be sent** (the buttons): edit only
  **`gui_web/templates/index.html`**. Each button is
  `<button class="cmd-btn safety-event" data-event="N">Label</button>`;
  `wireCommandForms()` in `app.js` wires up every such button generically
  by reading its `data-event`, so adding, removing, or relabeling a
  button is a pure HTML edit.
- **To change which safety levels the firmware can report** (and their
  display labels): edit `safetyLevelToString()` in
  **`gui_web/static/app.js`** to match the firmware's actual level
  numbers and meanings, and add/adjust the matching
  `.last-safety-readout.level-N` color rules in
  **`gui_web/static/style.css`** (one rule per level number; missing ones
  just fall back to the default text color, which is harmless but
  won't visually distinguish that level).
