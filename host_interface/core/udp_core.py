#!/usr/bin/env python3
## @file udp_core.py
#  @brief Shared UDP telemetry/command core for the ESP32 link.
#
#  @details
#  This module owns:
#    - the wire protocol (struct formats, packet building/parsing)
#    - the socket + background receiver thread
#    - a small callback/observer API for telemetry
#    - typed send_*() methods for every command
#
#  It intentionally knows nothing about GUIs or ROS2. Both the web GUI
#  (gui_web/app.py) and the ROS2 node (ros2_ws/.../bridge_node.py) import
#  this module and drive it through UdpClient. If you only ever change
#  one file when the ESP32-side protocol changes, it should be this one.

from __future__ import annotations

import json
import os
import socket
import struct
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, List, Optional, Tuple

# --------------------------------------------------------------------------
# Wire protocol
# --------------------------------------------------------------------------

## @name Wire protocol constants
#  @brief Framing/message-type constants shared with the firmware.
#  @{

## Fixed magic byte that must prefix every packet in either direction.
PROTO_MAGIC = 0xA5

## Host -> ESP32: register this host as the telemetry destination.
MSG_TYPE_HELLO = 0x00
## ESP32 -> host: a batch of telemetry samples.
MSG_TYPE_TELEMETRY = 0x01
## Host -> ESP32: trigger a safety event.
MSG_TYPE_CMD_SAFETY = 0x02
## Host -> ESP32: remove a queued step by id.
MSG_TYPE_CMD_REMOVE_STEP = 0x03
## Host -> ESP32: queue a wait step.
MSG_TYPE_CMD_WAIT = 0x04
## Host -> ESP32: unregister this host as the telemetry destination.
MSG_TYPE_GOODBYE = 0xFF
## @}

## Number of safety/control sample slots reserved per telemetry batch.
SAMPLES_PER_BATCH = 20
## Number of sequencer-step slots reserved per telemetry batch.
STEPS_PER_BATCH = 5

# Must match the packed C structs in lib/udp_interface/src/udp_interface.c exactly:
# little-endian, no padding (structs are __attribute__((packed))).

## @name struct.pack/unpack format strings
#  @brief Must stay byte-for-byte in sync with the firmware's packed C structs
#  (little-endian, no padding).
#  @{

## One safety sample: `timestamp_us` (uint32), `safety_level` (uint8).
SAFETY_SAMPLE_FMT = "<IB"
## One control sample: `timestamp_us` (uint32), `enc_A_pos, enc_B_pos` (2x float32).
CONTROL_SAMPLE_FMT = "<I2f"
## One sequencer step: `step_type, step_id` (uint8 each), `duration_ms` (uint32).
SEQUENCER_STEP_FMT = "<BBI"
## Telemetry packet header: `magic, msg_type` (uint8 each), `seq` (uint16),
#  `safety_cnt, control_cnt, steps_cnt` (uint8 each).
HEADER_FMT = "<BBHBBB"
## HELLO/GOODBYE packet: `magic, msg_type` (uint8 each), `seq` (uint16).
HELLO_FMT = "<BBH"
## Safety command packet: `magic, msg_type` (uint8 each), `seq` (uint16), `safety_event` (uint8).
SAFETY_FMT = "<BBHB"
## Wait command packet: `magic, msg_type` (uint8 each), `seq` (uint16), `duration_ms` (uint32).
WAIT_FMT = "<BBHI"
## Remove-step command packet: `magic, msg_type` (uint8 each), `seq` (uint16), `step_id` (uint8).
REMOVE_STEP_FMT = "<BBHB"
## @}

## @name Derived struct sizes (bytes), computed once at import time.
#  @{
SAFETY_SAMPLE_SIZE = struct.calcsize(SAFETY_SAMPLE_FMT)
CONTROL_SAMPLE_SIZE = struct.calcsize(CONTROL_SAMPLE_FMT)
SEQUENCER_STEP_SIZE = struct.calcsize(SEQUENCER_STEP_FMT)
HEADER_SIZE = struct.calcsize(HEADER_FMT)
## @}

# Sequencer step_type values (must match firmware).
## Sequencer step type: wait for a fixed duration.
STEP_TYPE_WAIT = 0


# --------------------------------------------------------------------------
# Config file (lets you store esp32_host/esp32_port instead of retyping them)
# --------------------------------------------------------------------------

## Default config file name searched for by find_config_path().
CONFIG_FILENAME = "esp32_config.json"
## Environment variable that can point directly at a config file.
CONFIG_ENV_VAR = "ESP32_CONFIG_PATH"


def find_config_path(explicit_path: Optional[str] = None) -> Optional[Path]:
    """!
    @brief Locate an esp32_config.json file using a fixed search order.

    @details
    Resolution order:
      1. explicit_path argument
      2. ESP32_CONFIG_PATH environment variable
      3. ./esp32_config.json (current working directory)
      4. <repo_root>/esp32_config.json (repo root = parent of this file's
         parent, i.e. host_interface/ when this file is at core/)

    @param explicit_path Optional explicit path that, if given, is checked
           first and takes priority over every other location.
    @return The first existing config file path found, or `None` if none
            of the candidate locations exist.
    """
    candidates = []
    if explicit_path:
        candidates.append(Path(explicit_path))
    env_path = os.environ.get(CONFIG_ENV_VAR)
    if env_path:
        candidates.append(Path(env_path))
    candidates.append(Path.cwd() / CONFIG_FILENAME)
    candidates.append(Path(__file__).resolve().parent.parent / CONFIG_FILENAME)

    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def load_config(explicit_path: Optional[str] = None) -> dict:
    """!
    @brief Load `{"esp32_host": ..., "esp32_port": ...}` from a JSON config file.

    @param explicit_path Forwarded to find_config_path(); see its docs for
           the search order.
    @return The parsed config dict, or `{}` if no config file is found --
            callers should fall back to CLI args / ROS2 params / hardcoded
            defaults in that case.
    """
    path = find_config_path(explicit_path)
    if path is None:
        return {}
    with open(path) as f:
        return json.load(f)


def resolve_host(host: str) -> str:
    """!
    @brief Resolve a hostname to an IPv4 address.

    @details
    Plain dotted IPv4 addresses are returned unchanged. Regular DNS
    hostnames and mDNS '.local' names (e.g. an ESP32 advertising itself as
    'esp32-robot.local') are resolved via the OS resolver -- note that
    '.local' resolution requires mDNS support to be set up on this machine
    (built in on macOS; on Linux needs avahi-daemon + libnss-mdns; on
    Windows needs Bonjour/iTunes or similar).

    @param host Dotted IPv4 address or hostname to resolve.
    @return The resolved IPv4 address as a string.
    @throws RuntimeError if `host` is not a valid IPv4 address and could
            not be resolved via DNS/mDNS.
    """
    try:
        socket.inet_aton(host)
        return host  # already a dotted IPv4 address
    except OSError:
        pass
    try:
        return socket.gethostbyname(host)
    except socket.gaierror as exc:
        raise RuntimeError(
            f"Could not resolve host '{host}': {exc}. If this is an mDNS "
            "'.local' name, make sure mDNS resolution is set up on this "
            "machine, or use a plain IP address / regular DNS hostname "
            "instead."
        ) from exc


# --------------------------------------------------------------------------
# Parsed data types
# --------------------------------------------------------------------------

@dataclass
class SafetySample:
    """!
    @brief One decoded safety sample from a telemetry batch.
    """
    ## ESP32-side timestamp of the sample, in microseconds since boot.
    timestamp_us: int
    ## Current safety level reported by the firmware.
    safety_level: int


@dataclass
class ControlSample:
    """!
    @brief One decoded control/pose sample from a telemetry batch.
    """
    ## ESP32-side timestamp of the sample, in microseconds since boot.
    timestamp_us: int
    ## Encoder A position.
    enc_A_pos: float
    ## Encoder B position.
    enc_B_pos: float


@dataclass
class SequencerStep:
    """!
    @brief One entry from the ESP32's sequencer queue.

    @details
    Only the fields relevant to `step_type` carry valid data; the rest are
    zero (e.g. for a STEP_TYPE_WAIT step only `duration_ms` is meaningful).
    """
    ## STEP_TYPE_MOVE_TO or STEP_TYPE_WAIT.
    step_type: int
    ## Firmware-assigned id of this step, used with send_remove_step().
    step_id: int
    ## Wait duration in milliseconds (valid for STEP_TYPE_WAIT).
    duration_ms: int


@dataclass
class TelemetryBatch:
    """!
    @brief One fully parsed telemetry packet from the ESP32.
    """
    ## Rolling 16-bit sequence number assigned by the firmware.
    seq: int
    ## Safety samples contained in this batch (possibly empty).
    safety_samples: List[SafetySample] = field(default_factory=list)
    ## Control samples contained in this batch (possibly empty).
    control_samples: List[ControlSample] = field(default_factory=list)
    ## Current snapshot of the sequencer queue (possibly empty).
    steps: List[SequencerStep] = field(default_factory=list)


@dataclass
class LinkStats:
    """!
    @brief Running link-quality counters maintained by UdpClient.
    """
    ## Total number of valid telemetry packets received so far.
    total_packets: int = 0
    ## Total number of safety + control samples received so far.
    total_samples: int = 0
    ## Estimated number of telemetry packets lost, inferred from sequence gaps.
    dropped_packets: int = 0
    ## Sequence number of the most recently received packet, or `None` before the first.
    last_seq: Optional[int] = None
    ## `time.time()` timestamp of the most recently received packet, or `None` before the first.
    last_rx_time: Optional[float] = None


# --------------------------------------------------------------------------
# Packet builders (host -> ESP32)
# --------------------------------------------------------------------------

class _SeqCounter:
    """!
    @brief Thread-safe 16-bit rolling sequence counter for outgoing packets.
    """

    def __init__(self):
        """! @brief Initialize the counter at 0. """
        self._lock = threading.Lock()
        self._seq = 0

    def next(self) -> int:
        """!
        @brief Atomically increment and return the next sequence number.
        @return The next sequence number, wrapped to the range [0, 0xFFFF].
        """
        with self._lock:
            self._seq = (self._seq + 1) & 0xFFFF
            return self._seq


def build_hello(seq: int) -> bytes:
    """!
    @brief Build a HELLO packet that registers this host with the ESP32.
    @param seq Outgoing sequence number (see _SeqCounter.next()).
    @return The packed HELLO packet bytes.
    """
    return struct.pack(HELLO_FMT, PROTO_MAGIC, MSG_TYPE_HELLO, seq)


def build_goodbye(seq: int) -> bytes:
    """!
    @brief Build a GOODBYE packet that unregisters this host from the ESP32.
    @param seq Outgoing sequence number (see _SeqCounter.next()).
    @return The packed GOODBYE packet bytes.
    """
    return struct.pack(HELLO_FMT, PROTO_MAGIC, MSG_TYPE_GOODBYE, seq)


def build_safety_cmd(seq: int, event: int) -> bytes:
    """!
    @brief Build a command packet that triggers a safety event on the ESP32.
    @param seq Outgoing sequence number (see _SeqCounter.next()).
    @param event Safety event code to trigger.
    @return The packed command packet bytes.
    """
    return struct.pack(SAFETY_FMT, PROTO_MAGIC, MSG_TYPE_CMD_SAFETY, seq, event)


def build_wait_cmd(seq: int, duration_ms: int) -> bytes:
    """!
    @brief Build a command packet that queues a wait sequencer step.
    @param seq Outgoing sequence number (see _SeqCounter.next()).
    @param duration_ms Wait duration in milliseconds.
    @return The packed command packet bytes.
    """
    return struct.pack(WAIT_FMT, PROTO_MAGIC, MSG_TYPE_CMD_WAIT, seq, duration_ms)


def build_remove_step_cmd(seq: int, step_id: int) -> bytes:
    """!
    @brief Build a command packet that removes a queued sequencer step.
    @param seq Outgoing sequence number (see _SeqCounter.next()).
    @param step_id Id of the step to remove (see SequencerStep.step_id).
    @return The packed command packet bytes.
    """
    return struct.pack(REMOVE_STEP_FMT, PROTO_MAGIC, MSG_TYPE_CMD_REMOVE_STEP, seq, step_id)


# --------------------------------------------------------------------------
# Packet parsing (ESP32 -> host)
# --------------------------------------------------------------------------

def parse_telemetry(data: bytes) -> Optional[TelemetryBatch]:
    """!
    @brief Parse a raw UDP payload into a TelemetryBatch.

    @details
    Validates the magic byte and message type, then decodes as many
    safety/control samples and sequencer steps as both the declared counts
    in the header and the actual buffer length allow (truncated/corrupt
    packets are handled gracefully by simply stopping early).

    @param data Raw bytes received from the socket.
    @return A populated TelemetryBatch, or `None` if `data` is too short,
            has the wrong magic byte, or is not a telemetry message.
    """
    if len(data) < HEADER_SIZE:
        return None
    magic, msg_type, seq, safety_cnt, control_cnt, steps_cnt = struct.unpack_from(
        HEADER_FMT, data, 0
    )
    if magic != PROTO_MAGIC or msg_type != MSG_TYPE_TELEMETRY:
        return None

    batch = TelemetryBatch(seq=seq)

    offset = HEADER_SIZE
    for _ in range(safety_cnt):
        if offset + SAFETY_SAMPLE_SIZE > len(data):
            break
        ts, level = struct.unpack_from(SAFETY_SAMPLE_FMT, data, offset)
        batch.safety_samples.append(SafetySample(ts, level))
        offset += SAFETY_SAMPLE_SIZE

    offset = HEADER_SIZE + SAMPLES_PER_BATCH * SAFETY_SAMPLE_SIZE
    for _ in range(control_cnt):
        if offset + CONTROL_SAMPLE_SIZE > len(data):
            break
        ts, enc_A_pos, enc_B_pos = struct.unpack_from(CONTROL_SAMPLE_FMT, data, offset)
        batch.control_samples.append(ControlSample(ts, enc_A_pos, enc_B_pos))
        offset += CONTROL_SAMPLE_SIZE

    offset = HEADER_SIZE + SAMPLES_PER_BATCH * (SAFETY_SAMPLE_SIZE + CONTROL_SAMPLE_SIZE)
    for _ in range(steps_cnt):
        if offset + SEQUENCER_STEP_SIZE > len(data):
            break
        step_type, step_id, duration_ms = struct.unpack_from(
            SEQUENCER_STEP_FMT, data, offset
        )
        batch.steps.append(SequencerStep(step_type, step_id, duration_ms))
        offset += SEQUENCER_STEP_SIZE

    return batch


# --------------------------------------------------------------------------
# Client
# --------------------------------------------------------------------------

## Type alias for a telemetry subscriber callback: `callback(batch: TelemetryBatch) -> None`.
TelemetryCallback = Callable[[TelemetryBatch], None]


class UdpClient:
    """!
    @brief Owns the UDP link to a single ESP32.

    @details
    Sends HELLO/GOODBYE/commands, and runs a background thread that parses
    incoming telemetry and dispatches it to registered callbacks.

    Usage:
    @code
        client = UdpClient("192.168.1.50")          # or a hostname:
        client = UdpClient("esp32-robot.local")
        client.on_telemetry(my_callback)
        client.start()
        ...
        client.send_move_to(1.0, 0.5, 0.0)
        ...
        client.stop()
    @endcode
    """

    def __init__(
        self,
        esp32_host: str,
        port: int = 3333,
        local_port: Optional[int] = None,
        recv_bufsize: int = 4096,
        socket_timeout_s: float = 1.0,
    ):
        """!
        @brief Construct a client. Does not open a socket or send anything
               until start() is called.

        @param esp32_host Dotted IPv4 address or hostname (incl. mDNS
               '.local' names) of the ESP32. Resolved once, in start().
        @param port UDP port the ESP32 listens on.
        @param local_port Local UDP port to bind to. Defaults to the same
               value as `port` (matching the firmware's expectation of a
               symmetric port on both ends).
        @param recv_bufsize Maximum UDP payload size to read per `recvfrom()`.
        @param socket_timeout_s Receive timeout, in seconds; controls how
               responsively the receiver thread notices `stop()` being called.
        """
        # esp32_host may be a dotted IPv4 address or a hostname (incl. mDNS
        # '.local' names) -- resolved once, in start(). self.dest stays
        # None until then; use self.esp32_host if you need the original
        # unresolved value.
        self.esp32_host = esp32_host
        self.port = port
        ## Resolved `(ip, port)` destination tuple; `None` until start() runs.
        self.dest: Optional[Tuple[str, int]] = None
        self._local_port = local_port if local_port is not None else port
        self._recv_bufsize = recv_bufsize
        self._socket_timeout_s = socket_timeout_s

        self._sock: Optional[socket.socket] = None
        self._rx_thread: Optional[threading.Thread] = None
        self._running = False

        self._seq_counter = _SeqCounter()
        self._callbacks: List[TelemetryCallback] = []
        self._callbacks_lock = threading.Lock()

        ## Running link-quality counters; see LinkStats.
        self.stats = LinkStats()
        self._stats_lock = threading.Lock()

    # -- lifecycle ---------------------------------------------------

    def start(self) -> None:
        """!
        @brief Resolve the ESP32 host, open the socket, start the receiver
               thread, and send the initial HELLO packet.

        @details
        Safe to call more than once; subsequent calls are a no-op while
        already running.

        @throws RuntimeError if `esp32_host` cannot be resolved to an IP
                address.
        """
        if self._running:
            return
        try:
            resolved_ip = resolve_host(self.esp32_host)
        except Exception as exc:
             raise RuntimeError(
                 f"Failed to resolve ESP32 host '{self.esp32_host}': {exc}"
             ) from exc
        self.dest = (resolved_ip, self.port)
        if resolved_ip != self.esp32_host:
            print(f"[udp_core] resolved {self.esp32_host} -> {resolved_ip}")

        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.bind(("0.0.0.0", self._local_port))
        self._sock.settimeout(self._socket_timeout_s)
        self._running = True

        self._rx_thread = threading.Thread(target=self._recv_loop, daemon=True)
        self._rx_thread.start()

        self._send(build_hello(self._seq_counter.next()))

    def stop(self) -> None:
        """!
        @brief Send a GOODBYE packet, stop the receiver thread, and close
               the socket.

        @details
        Safe to call more than once, and safe to call even if start() was
        never called (in which case it's a no-op).
        """
        if not self._running:
            return
        try:
            self._send(build_goodbye(self._seq_counter.next()))
        except OSError:
            pass
        self._running = False
        if self._rx_thread is not None:
            self._rx_thread.join(timeout=2.0)
        if self._sock is not None:
            self._sock.close()
            self._sock = None

    def __enter__(self):
        """! @brief Context-manager entry: calls start() and returns self. """
        self.start()
        return self

    def __exit__(self, exc_type, exc, tb):
        """! @brief Context-manager exit: calls stop(). """
        self.stop()

    # -- telemetry subscription ---------------------------------------

    def on_telemetry(self, callback: TelemetryCallback) -> None:
        """!
        @brief Register a callback invoked for every received telemetry batch.
        @param callback Callable of the form `callback(batch: TelemetryBatch) -> None`,
               invoked from the receiver thread (not the caller's thread).
        """
        with self._callbacks_lock:
            self._callbacks.append(callback)

    def remove_telemetry_callback(self, callback: TelemetryCallback) -> None:
        """!
        @brief Unregister a previously registered telemetry callback.
        @param callback The exact callable object passed to on_telemetry() earlier.
        """
        with self._callbacks_lock:
            if callback in self._callbacks:
                self._callbacks.remove(callback)

    # -- commands ------------------------------------------------------

    def send_safety(self, event: int) -> None:
        """!
        @brief Send a command that triggers a safety event on the ESP32.
        @param event Safety event code to trigger.
        """
        self._send(build_safety_cmd(self._seq_counter.next(), event))

    def send_wait(self, duration_ms: int) -> None:
        """!
        @brief Send a command that queues a wait sequencer step.
        @param duration_ms Wait duration in milliseconds.
        """
        self._send(build_wait_cmd(self._seq_counter.next(), duration_ms))

    def send_remove_step(self, step_id: int) -> None:
        """!
        @brief Send a command that removes a queued sequencer step.
        @param step_id Id of the step to remove (see SequencerStep.step_id).
        """
        self._send(build_remove_step_cmd(self._seq_counter.next(), step_id))

    # -- internals -------------------------------------------------------

    def _send(self, packet: bytes) -> None:
        """!
        @brief Send a raw packet to the resolved ESP32 destination.
        @param packet Fully built packet bytes (see the build_*() functions).
        @throws RuntimeError if start() has not been called yet.
        """
        if self._sock is None:
            raise RuntimeError("UdpClient.start() must be called before sending")
        self._sock.sendto(packet, self.dest)

    def _recv_loop(self) -> None:
        """!
        @brief Background receiver loop: reads packets, parses telemetry,
               updates stats, and dispatches callbacks. Runs until `stop()`
               clears `self._running` or the socket errors out.
        """
        assert self._sock is not None
        while self._running:
            try:
                data, _ = self._sock.recvfrom(self._recv_bufsize)
            except socket.timeout:
                continue
            except OSError:
                break

            batch = parse_telemetry(data)
            if batch is None:
                continue

            self._update_stats(batch)

            with self._callbacks_lock:
                callbacks = list(self._callbacks)
            for cb in callbacks:
                try:
                    cb(batch)
                except Exception as exc:  # noqa: BLE001 - never let a bad callback kill the thread
                    print(f"[udp_core] telemetry callback raised: {exc!r}")

    def _update_stats(self, batch: TelemetryBatch) -> None:
        """!
        @brief Update LinkStats from a newly received batch (packet/sample
               counts, and an estimate of dropped packets from sequence gaps).
        @param batch The just-parsed TelemetryBatch.
        """
        with self._stats_lock:
            s = self.stats
            if s.last_seq is not None:
                expected = (s.last_seq + 1) & 0xFFFF
                if batch.seq != expected:
                    s.dropped_packets += (batch.seq - expected) & 0xFFFF
            s.last_seq = batch.seq
            s.total_packets += 1
            s.total_samples += len(batch.safety_samples) + len(batch.control_samples)
            s.last_rx_time = time.time()
