/**
 * @file app.js
 * @brief Host side ESP32 Telemetry Console frontend.
 *
 * Consumes the /events SSE stream from app.py and renders it as
 * runtime-configurable Chart.js time-series panels, plus sends commands
 * via simple fetch() POSTs.
 */

/**
 * @brief Registry of plottable telemetry signals.
 * @details Each entry maps a signal key (as used in the /events JSON
 * payload) to its display label, chart color (read from a CSS custom
 * property), and which part of a telemetry batch it comes from
 * ("control" samples or the "safety" sample).
 */
const SIGNALS = {
  enc_A_pos:      { label: "Encoder A Position", color: getVar("--sig-enc-A"),  source: "control" },
  enc_B_pos:      { label: "Encoder B Position", color: getVar("--sig-enc-B"),  source: "control" },
  safety_level:   { label: "Safety Level",       color: getVar("--sig-safety"), source: "safety" },
};

/**
 * @brief Read a CSS custom property's current value from :root.
 * @param {string} name CSS custom property name, e.g. "--sig-x".
 * @returns {string} The trimmed property value.
 */
function getVar(name) {
  return getComputedStyle(document.documentElement).getPropertyValue(name).trim();
}

/** @brief Seconds of history kept client-side per signal, regardless of any single panel's window. */
const MAX_BUFFER_SECONDS = 300; // keep 5 min of history client-side regardless of panel window
/** @brief How often (ms) chart panels are redrawn from the rolling buffers. */
const RENDER_INTERVAL_MS = 300;

/**
 * @brief Rolling per-signal sample buffers.
 * @details Maps each signal key to `{ t: number[], v: number[] }`, where
 * `t` is seconds since session start (see toSessionSeconds()) and `v` is
 * the corresponding sample value. Both arrays are kept in sync and capped
 * to MAX_BUFFER_SECONDS by pushSample().
 */
const buffers = {};
for (const key of Object.keys(SIGNALS)) buffers[key] = { t: [], v: [] };

/** @brief First control/safety sample `t_us` seen this session; used as the time origin. */
let originUs = null; // first control/safety sample timestamp_us seen this session
/** @brief `performance.now()` at page load; currently unused beyond documenting session start. */
const sessionStart = performance.now();

/**
 * @brief Convert a firmware microsecond timestamp to seconds relative to
 *        the first sample seen this session.
 * @details Also compensates for a single wraparound of the firmware's
 * 32-bit `micros()` counter.
 * @param {number} t_us Firmware timestamp, in microseconds since boot.
 * @returns {number} Elapsed seconds since the first sample of this session.
 */
function toSessionSeconds(t_us) {
  if (originUs === null) originUs = t_us;
  // handle uint32 wraparound of the firmware's micros() counter
  let deltaUs = t_us - originUs;
  if (deltaUs < -(2 ** 31)) deltaUs += 2 ** 32;
  return deltaUs / 1e6;
}

/**
 * @brief Append a sample to a signal's rolling buffer and evict anything
 *        older than MAX_BUFFER_SECONDS.
 * @param {string} key Signal key, e.g. "x" or "safety_level".
 * @param {number} tSec Sample time, in seconds since session start.
 * @param {number} value Sample value.
 */
function pushSample(key, tSec, value) {
  const buf = buffers[key];
  buf.t.push(tSec);
  buf.v.push(value);
  const cutoff = tSec - MAX_BUFFER_SECONDS;
  while (buf.t.length && buf.t[0] < cutoff) {
    buf.t.shift();
    buf.v.shift();
  }
}

// -- panel state -------------------------------------------------------

/** @brief Monotonically increasing id generator for new chart panels. */
let nextPanelId = 1;
/**
 * @brief Live list of chart panel state objects.
 * @details Each entry has the shape
 * `{ id, title, signals: Set<string>, windowSec, chart: Chart }`.
 */
const panels = []; // { id, title, signals: Set<string>, windowSec, chart: Chart }

/**
 * @brief Create the three default panels shown on page load
 *        (Pose, Velocity, Safety).
 */
function defaultPanels() {
  addPanel("Encoder Position", ["enc_A_pos", "enc_B_pos"], 30);
  addPanel("Safety", ["safety_level"], 30);
}

/**
 * @brief Create a new chart panel, add it to #panels, and render its DOM/chart.
 * @param {string} title Initial panel title (editable afterwards).
 * @param {string[]} signalKeys Initial set of signal keys to plot (see SIGNALS).
 * @param {number} windowSec Initial visible time window, in seconds.
 */
function addPanel(title, signalKeys, windowSec) {
  const panel = {
    id: nextPanelId++,
    title: title || "Plot",
    signals: new Set(signalKeys || ["x"]),
    windowSec: windowSec || 30,
    chart: null,
  };
  panels.push(panel);
  renderPanel(panel);
}

/**
 * @brief Destroy a panel's chart, remove its DOM card, and drop it from #panels.
 * @param {number} id Id of the panel to remove.
 */
function removePanel(id) {
  const idx = panels.findIndex((p) => p.id === id);
  if (idx === -1) return;
  panels[idx].chart?.destroy();
  document.getElementById(`chart-card-${id}`)?.remove();
  panels.splice(idx, 1);
}

/**
 * @brief Build the DOM card for a panel (title input, window input, signal
 *        toggle chips, canvas) and wire up its event listeners and Chart.js instance.
 * @param {object} panel Panel state object, as created by addPanel().
 */
function renderPanel(panel) {
  const column = document.getElementById("charts-column");
  const addBtn = document.getElementById("add-chart-btn");

  const card = document.createElement("div");
  card.className = "chart-card";
  card.id = `chart-card-${panel.id}`;

  const toggles = Object.keys(SIGNALS)
    .map((key) => {
      const active = panel.signals.has(key) ? "active" : "";
      const color = SIGNALS[key].color;
      const style = panel.signals.has(key) ? `background:${color};border-color:${color}` : "";
      return `<span class="signal-toggle ${active}" data-key="${key}" style="${style}">${SIGNALS[key].label}</span>`;
    })
    .join("");

  card.innerHTML = `
    <div class="chart-card-head">
      <input class="title-input" value="${panel.title}" />
      <div class="chart-controls">
        <label>window <input type="number" class="window-input" value="${panel.windowSec}" min="2" max="${MAX_BUFFER_SECONDS}" />s</label>
        <button class="remove-chart-btn" title="Remove plot">&times;</button>
      </div>
    </div>
    <div class="signal-toggles">${toggles}</div>
    <div class="chart-canvas-wrap"><canvas></canvas></div>
  `;

  column.insertBefore(card, addBtn);

  card.querySelector(".title-input").addEventListener("input", (e) => {
    panel.title = e.target.value;
  });
  card.querySelector(".window-input").addEventListener("change", (e) => {
    panel.windowSec = Math.max(2, Number(e.target.value) || panel.windowSec);
  });
  card.querySelector(".remove-chart-btn").addEventListener("click", () => removePanel(panel.id));
  card.querySelectorAll(".signal-toggle").forEach((el) => {
    el.addEventListener("click", () => {
      const key = el.dataset.key;
      if (panel.signals.has(key)) {
        panel.signals.delete(key);
        el.classList.remove("active");
        el.style.background = "";
        el.style.borderColor = "";
      } else {
        panel.signals.add(key);
        el.classList.add("active");
        el.style.background = SIGNALS[key].color;
        el.style.borderColor = SIGNALS[key].color;
      }
      rebuildChart(panel);
    });
  });

  const ctx = card.querySelector("canvas").getContext("2d");
  panel.chart = new Chart(ctx, {
    type: "line",
    data: { datasets: [] },
    options: chartOptions(),
  });
  rebuildChart(panel);
}

/**
 * @brief Build the shared Chart.js options object used by every panel
 *        (dark theme colors, no animation/points, linear time x-axis).
 * @returns {object} A Chart.js `options` config.
 */
function chartOptions() {
  const muted = getVar("--text-muted");
  const border = getVar("--border");
  return {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    parsing: false,
    normalized: true,
    scales: {
      x: {
        type: "linear",
        ticks: { color: muted, font: { family: "monospace", size: 10 } },
        grid: { color: border },
      },
      y: {
        ticks: { color: muted, font: { family: "monospace", size: 10 } },
        grid: { color: border },
      },
    },
    plugins: {
      legend: { display: false },
    },
    elements: {
      point: { radius: 0 },
      line: { borderWidth: 1.5, tension: 0.15 },
    },
  };
}

/**
 * @brief Rebuild a panel's Chart.js datasets to match its current signal
 *        selection (called whenever the selection changes).
 * @param {object} panel Panel state object.
 */
function rebuildChart(panel) {
  panel.chart.data.datasets = [...panel.signals].map((key) => ({
    label: SIGNALS[key].label,
    borderColor: SIGNALS[key].color,
    backgroundColor: SIGNALS[key].color,
    data: [],
  }));
  panel.chart.update("none");
}

/**
 * @brief Redraw every panel from the current buffer contents, clipped to
 *        each panel's own time window. Called on a fixed interval
 *        (see RENDER_INTERVAL_MS), not on every incoming sample, to keep
 *        rendering cheap.
 */
function refreshCharts() {
  const latestTimes = Object.values(buffers)
    .filter((b) => b.t.length)
    .map((b) => b.t[b.t.length - 1]);
  const now = latestTimes.length ? Math.max(...latestTimes) : 0;

  for (const panel of panels) {
    if (!panel.chart) continue;
    const from = now - panel.windowSec;
    panel.chart.data.datasets.forEach((ds) => {
      const key = Object.keys(SIGNALS).find((k) => SIGNALS[k].label === ds.label);
      const buf = buffers[key];
      const points = [];
      for (let i = buf.t.length - 1; i >= 0; i--) {
        if (buf.t[i] < from) break;
        points.unshift({ x: buf.t[i], y: buf.v[i] });
      }
      ds.data = points;
    });
    panel.chart.options.scales.x.min = from;
    panel.chart.options.scales.x.max = now;
    panel.chart.update("none");
  }
}

// -- sequencer step track -----------------------------------------------

/**
 * @brief Render the sequencer queue as a row of step cards in #step-track.
 * @param {Array<object>} steps Array of step objects from the latest
 *        telemetry batch, each `{ step_type, step_id, x, y, phi, duration_ms }`.
 */
function renderSteps(steps) {
  const track = document.getElementById("step-track");
  track.innerHTML = steps
    .map((s) => {
      switch (s.step_type) {
        case 0: // wait
          return `<div class="step-card type-wait">
            <div class="step-id">${s.step_id}</div>
            <div class="step-type">wait</div>
            <div class="step-detail">dur ${s.duration_ms}ms</div>
          </div>`;
        default: // unrecognized step type
          return `<div class="step-card type-unknown">
            <div class="step-id">${s.step_id}</div>
            <div class="step-type">Unknown</div>
            <div class="step-detail">Unknown step type</div>
          </div>`;
      }
    })
    .join("");
}

// -- SSE ingestion --------------------------------------------------------

/**
 * @brief Prepend a timestamped line to the command log panel, capped at 50 lines.
 * @param {string} text Log message text.
 */
function logLine(text) {
  const log = document.getElementById("cmd-log");
  const line = document.createElement("div");
  const t = new Date().toLocaleTimeString();
  line.textContent = `[${t}] ${text}`;
  log.prepend(line);
  while (log.childElementCount > 50) log.removeChild(log.lastChild);
}

/**
 * @brief Map a numeric firmware safety_level to a human-readable label.
 * @param {number} level Safety level reported by the firmware.
 * @returns {string} Human-readable label, or "Unknown (<level>)" for
 *          unrecognized values.
 */
function safetyLevelToString(level) {
  switch (level) {
    case 0: return "System Off";
    case 1: return "Initializing";
    case 2: return "Shutting Down";
    case 3: return "Emergency";
    case 4: return "Ready";
    case 5: return "Active";
    default: return `Unknown (${level})`;
  }
}

/** @brief `performance.now()` timestamp of the last received SSE message; drives the connection-health dot. */
let lastRxTime = 0;

/**
 * @brief Open the /events SSE stream and wire it up to update buffers,
 *        the safety readout, the sequencer queue display, and the stats bar
 *        for every incoming telemetry batch.
 */
function connectStream() {
  const es = new EventSource("/events");
  es.onmessage = (evt) => {
    lastRxTime = performance.now();
    const batch = JSON.parse(evt.data);

    for (const s of batch.safety_samples) {
      pushSample("safety_level", toSessionSeconds(s.t_us), s.safety_level);
    }
    for (const c of batch.control_samples) {
      const tSec = toSessionSeconds(c.t_us);
      pushSample("enc_A_pos", tSec, c.enc_A_pos);
      pushSample("enc_B_pos", tSec, c.enc_B_pos);
    }

    if (batch.safety_samples.length) {
      const last = batch.safety_samples[batch.safety_samples.length - 1];
      const el = document.getElementById("last-safety-value");
      el.textContent = last.safety_level + " - " + safetyLevelToString(last.safety_level);
      el.className = `last-safety-readout level-${last.safety_level}`;
    }

    renderSteps(batch.steps);

    document.getElementById("stat-packets").textContent = batch.stats.total_packets;
    document.getElementById("stat-samples").textContent = batch.stats.total_samples;
    document.getElementById("stat-dropped").textContent = batch.stats.dropped_packets;
    document.getElementById("stat-seq").textContent = batch.seq;
  };
  es.onerror = () => {
    document.getElementById("conn-dot").classList.remove("live");
  };
}

/**
 * @brief Start a periodic check that toggles the connection-status dot
 *        based on how recently a telemetry batch was received.
 */
function watchConnectionHealth() {
  setInterval(() => {
    const dot = document.getElementById("conn-dot");
    const alive = performance.now() - lastRxTime < 3000;
    dot.classList.toggle("live", alive);
  }, 500);
}

// -- command forms --------------------------------------------------------

/**
 * @brief POST a command body to a Flask command endpoint and log the result.
 * @param {string} path API path, e.g. "/api/command/move_to".
 * @param {object} body JSON-serializable request body.
 * @returns {Promise<void>} Resolves once the request completes (success or failure);
 *          errors are caught and logged rather than thrown.
 */
async function postCommand(path, body) {
  try {
    const res = await fetch(path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body),
    });
    const data = await res.json();
    logLine(`${path} -> ${JSON.stringify(data.sent)}`);
  } catch (err) {
    logLine(`${path} FAILED: ${err}`);
  }
}

/**
 * @brief Wire up all command UI: safety event buttons and the move-to/wait/
 *        remove-step forms, each posting to its corresponding API endpoint.
 */
function wireCommandForms() {
  document.querySelectorAll(".cmd-btn[data-event]").forEach((btn) => {
    btn.addEventListener("click", () => {
      postCommand("/api/command/safety", { event: Number(btn.dataset.event) });
    });
  });

  document.getElementById("form-wait").addEventListener("submit", (e) => {
    e.preventDefault();
    const f = new FormData(e.target);
    postCommand("/api/command/wait", {
      duration_ms: Number(f.get("duration_ms"))
    });
    e.target.reset();
  });

  document.getElementById("form-remove-step").addEventListener("submit", (e) => {
    e.preventDefault();
    const f = new FormData(e.target);
    postCommand("/api/command/remove_step", { step_id: Number(f.get("step_id")) });
    e.target.reset();
  });
}

// -- boot --------------------------------------------------------------

document.getElementById("add-chart-btn").addEventListener("click", () => {
  addPanel("New plot", ["x"], 30);
});

defaultPanels();
wireCommandForms();
connectStream();
watchConnectionHealth();
setInterval(refreshCharts, RENDER_INTERVAL_MS);
