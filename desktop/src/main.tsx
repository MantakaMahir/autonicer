import React, { useEffect, useRef, useState } from "react";
import { createRoot } from "react-dom/client";
import { core } from "./lib/core";
import type {
  MemoryStatus,
  PagerResult,
  ProcessInfo,
  SystemStatus,
} from "./types";
import "./styles.css";
const pages = [
  ["dashboard", "Overview"],
  ["processes", "Processes"],
  ["memory", "Memory"],
  ["paging", "Paging Lab"],
  ["policies", "Policies"],
  ["settings", "Settings"],
];
function Badge({
  children,
  tone = "neutral",
}: {
  children: React.ReactNode;
  tone?: string;
}) {
  return <span className={`badge ${tone}`}>{children}</span>;
}
function Metric({
  label,
  value,
  detail,
  tone,
}: {
  label: string;
  value: React.ReactNode;
  detail: string;
  tone?: string;
}) {
  return (
    <div className="metric">
      <span>{label}</span>
      <strong className={tone?.toLowerCase()}>{value}</strong>
      <small>{detail}</small>
    </div>
  );
}
function App() {
  const [page, setPage] = useState("dashboard"),
    [status, setStatus] = useState<SystemStatus | null>(null),
    [memory, setMemory] = useState<MemoryStatus | null>(null),
    [processes, setProcesses] = useState<ProcessInfo[]>([]),
    [history, setHistory] = useState(""),
    [cpuHistory, setCpuHistory] = useState<number[]>([]),
    [connected, setConnected] = useState(false),
    [error, setError] = useState("");
  const refresh = async () => {
    try {
      const [s, m, p, h] = await Promise.all([
        core.status(),
        core.memory(),
        core.processes(),
        core.history(),
      ]);
      setStatus(s);
      setMemory(m);
      setProcesses(p);
      setHistory(h);
      setCpuHistory((x) => [...x, s.cpuPercent].slice(-60));
      setConnected(true);
      setError("");
    } catch (e) {
      setConnected(false);
      setError(String(e));
    }
  };
  useEffect(() => {
    refresh();
    const id = setInterval(refresh, 2500);
    return () => clearInterval(id);
  }, []);
  return (
    <div className="shell">
      <aside>
        <div className="brand">
          <div className="brand-mark">A</div>
          <div>
            <strong>AutoNicer</strong>
            <small>Linux control plane</small>
          </div>
        </div>
        <nav>
          {pages.map(([id, label]) => (
            <button
              className={page === id ? "active" : ""}
              onClick={() => setPage(id)}
              key={id}
            >
              {label}
            </button>
          ))}
        </nav>
        <div className="side-bottom">
          <div className="connection">
            <i className={connected ? "online" : ""} />
            <div>
               <b>
                 {core.isDesktopShell
                   ? connected
                     ? "Core connected"
                     : "Core unavailable"
                   : "Browser preview"}
               </b>
               <small>
                 {core.isDesktopShell
                    ? "Controller running"
                    : "Live data unavailable"}
               </small>
            </div>
          </div>
        </div>
      </aside>
      <main>
        <header>
          <div>
            <p className="eyebrow">LOAD-AWARE BACKGROUND JOB CONTROLLER</p>
            <h1>{pages.find((x) => x[0] === page)?.[1]}</h1>
          </div>
          <div className="header-actions">
            <Badge tone="green">MONITORING</Badge>
          </div>
        </header>
        {error && <div className="error">{error}</div>}
        {page === "dashboard" && (
          <Dashboard
            status={status}
            processes={processes}
            history={history}
            cpuHistory={cpuHistory}
          />
        )}{" "}
        {page === "processes" && (
          <Processes processes={processes} refresh={refresh} />
        )}{" "}
        {page === "memory" && <Memory memory={memory} />}{" "}
        {page === "paging" && <Paging />}
        {page === "policies" && <Policies />}
        {page === "settings" && <Settings connected={connected} />}
      </main>
    </div>
  );
}
function Dashboard({
  status,
  processes,
  history,
  cpuHistory,
}: {
  status: SystemStatus | null;
  processes: ProcessInfo[];
  history: string;
  cpuHistory: number[];
}) {
  const cpu = status?.cpuPercent ?? 0,
    state = status?.loadState ?? "NORMAL",
    points = (cpuHistory.length ? cpuHistory : [cpu])
      .map(
        (v, i, a) =>
          `${a.length < 2 ? 0 : (i * 800) / (a.length - 1)},${210 - v * 1.9}`,
      )
      .join(" ");
  return (
    <>
      <section className="metrics">
        <Metric
          label="CPU utilization"
          value={`${cpu.toFixed(1)}%`}
          detail="fresh /proc/stat sample"
          tone={state}
        />
        <Metric
          label="Controller state"
          value={state}
          detail="C-core decision state"
          tone={state}
        />
        <Metric
          label="Registered processes"
          value={processes.length}
          detail="registry"
        />
        <Metric
          label="Protected"
          value={processes.filter((p) => p.protected).length}
          detail="excluded"
        />
      </section>
      <section className="grid-main">
        <div className="panel chart-panel">
          <div className="panel-head">
            <div>
              <p className="eyebrow">LIVE OS TELEMETRY</p>
              <h2>CPU utilization</h2>
            </div>
          </div>
          <div className="chart">
            <svg viewBox="0 0 800 220" preserveAspectRatio="none">
              <polyline
                points={points}
                fill="none"
                stroke="currentColor"
                strokeWidth="3"
              />
            </svg>
          </div>
        </div>
        <div className="panel decision">
          <p className="eyebrow">POLICY OUTPUT</p>
          <h2>
            {state === "NORMAL"
              ? "No intervention required"
              : "Safe candidate evaluation active"}
          </h2>
          <p className="safety-note">
            The C core remains authoritative for all decisions.
          </p>
        </div>
      </section>
      <section className="panel">
        <p className="eyebrow">AUDIT TRAIL</p>
        <pre>
          {history.split("\n").filter(Boolean).slice(-5).join("\n") ||
            "No actions yet."}
        </pre>
      </section>
    </>
  );
}
function Processes({
  processes,
  refresh,
}: {
  processes: ProcessInfo[];
  refresh: () => void;
}) {
  const [actionError, setActionError] = useState("");
  const [query, setQuery] = useState("");
  const [selectedPid, setSelectedPid] = useState<number | null>(null);
  const normalizedQuery = query.trim().toLowerCase();
  const filtered = processes.filter(
    (p) =>
      !normalizedQuery ||
      p.name.toLowerCase().includes(normalizedQuery) ||
      String(p.pid).includes(normalizedQuery),
  );
  const selected = processes.find((p) => p.pid === selectedPid);
  const act = async (command: string, pid: number, classification?: string) => {
    try {
      await core.command(command, pid, classification);
      setActionError("");
      await refresh();
    } catch (e) {
      setActionError(String(e));
    }
  };
  return (
    <section className="panel process-panel">
      <div className="panel-head">
        <div>
          <p className="eyebrow">LIVE PROCESS DATA AND CONTROLS</p>
            <h2>Running user processes</h2>
        </div>
        <Badge>{processes.length}</Badge>
      </div>
      <label className="process-search">
        Search by process name or PID
        <input
          value={query}
          placeholder="e.g. cpu_hog or 1234"
          onChange={(e) => setQuery(e.target.value)}
        />
      </label>
      {actionError && <div className="error">{actionError}</div>}
      {selected && (
        <div className="selected-process">
          <div>
            <span className="eyebrow">SELECTED PROCESS</span>
            <b>{selected.name}</b>
            <small>
              PID {selected.pid} | {selected.classification} | RSS{" "}
              {(selected.rssKb / 1024).toFixed(1)}M
            </small>
          </div>
          <span className="mono">{selected.cpuPercent.toFixed(1)}% CPU</span>
        </div>
      )}
      <div className="table-wrap">
        <table>
          <thead>
            <tr>
              <th>PID</th>
              <th>Process</th>
              <th>CPU</th>
              <th>RSS</th>
              <th>Swap</th>
              <th>Class</th>
              <th>Protection</th>
              <th>Controls</th>
            </tr>
          </thead>
          <tbody>
            {filtered.map((p) => (
              <tr
                className={selectedPid === p.pid ? "selected" : ""}
                key={p.pid}
                onClick={() => setSelectedPid(p.pid)}
              >
                <td className="mono">{p.pid}</td>
                <td>
                  <b>{p.name}</b>
                </td>
                <td className="mono">{p.cpuPercent.toFixed(1)}%</td>
                <td className="mono">{(p.rssKb / 1024).toFixed(1)}M</td>
                <td className="mono">{(p.swapKb / 1024).toFixed(1)}M</td>
                <td>
                  <Badge
                    tone={
                      p.classification === "BACKGROUND"
                        ? "blue"
                        : p.classification === "CRITICAL"
                          ? "red"
                          : "neutral"
                    }
                  >
                    {p.classification}
                  </Badge>
                </td>
                <td>
                  {p.protected ? <Badge tone="purple">PROTECTED</Badge> : "—"}
                </td>
                <td>
                  <div className="button-grid">
                    <button
                      disabled={!core.isDesktopShell}
                      onClick={() => act("classify", p.pid, "normal")}
                    >
                      Normal
                    </button>
                    <button
                      disabled={!core.isDesktopShell}
                      onClick={() => act("classify", p.pid, "background")}
                    >
                      Background
                    </button>
                    <button
                      disabled={!core.isDesktopShell}
                      onClick={() => act("classify", p.pid, "critical")}
                    >
                      Critical
                    </button>
                    <button
                      disabled={!core.isDesktopShell}
                      onClick={() =>
                        act(p.protected ? "unprotect" : "protect", p.pid)
                      }
                    >
                      {p.protected ? "Unprotect" : "Protect"}
                    </button>
                    <button
                      disabled={!core.isDesktopShell || !p.priorityChanged}
                      onClick={() => act("restore", p.pid)}
                    >
                      Restore
                    </button>
                    <button
                      disabled={!core.isDesktopShell || !p.paused}
                      onClick={() => act("resume", p.pid)}
                    >
                      Resume
                    </button>
                    <button
                      className="danger-button"
                      disabled={
                        !core.isDesktopShell ||
                        p.classification !== "BACKGROUND" ||
                        p.protected
                      }
                      onClick={() => {
                        if (
                          window.confirm(
                            `Terminate ${p.name} (PID ${p.pid})? This cannot be undone.`,
                          )
                        )
                          void act("kill", p.pid);
                      }}
                    >
                      Kill
                    </button>
                  </div>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
        {!filtered.length && (
          <div className="empty table-empty">
            <b>{query ? "No matching processes" : "No running user processes"}</b>
            <span>
              {query
                ? "Try a different process name or PID."
                : "Processes will appear when the core refreshes /proc."}
            </span>
          </div>
        )}
      </div>
    </section>
  );
}
function Memory({ memory }: { memory: MemoryStatus | null }) {
  if (!memory)
    return (
      <section className="panel empty-page">
        Waiting for OS memory data.
      </section>
    );
  return (
    <>
      <section className="metrics">
        <Metric
          label="Memory pressure"
          value={memory.memoryState}
          detail="/proc/meminfo"
          tone={memory.memoryState}
        />
        <Metric
          label="Available RAM"
          value={`${(memory.availableKb / 1024).toFixed(0)} MB`}
          detail={`${memory.availablePercent.toFixed(1)}%`}
        />
        <Metric
          label="Swap used"
          value={`${(memory.swapUsedKb / 1024).toFixed(0)} MB`}
          detail="/proc/meminfo"
        />
        <Metric
          label="Major faults/s"
          value={memory.majorFaultsPerSecond.toFixed(1)}
          detail="/proc/vmstat"
        />
      </section>
      <section className="panel">
        <div className="panel-head">
          <div>
            <p className="eyebrow">LIVE LINUX MEMORY</p>
            <h2>Freshness and counters</h2>
          </div>
        </div>
        <p className="sample-time">
          Latest OS sample:{" "}
          {new Date(memory.timestamp * 1000).toLocaleTimeString()}
        </p>
        <div className="counter-grid">
          <Metric
            label="Minor faults/s"
            value={memory.minorFaultsPerSecond.toFixed(1)}
            detail="kernel counter delta"
          />
          <Metric
            label="Swap in/s"
            value={memory.swapInPerSecond.toFixed(1)}
            detail="kernel counter delta"
          />
          <Metric
            label="Swap out/s"
            value={memory.swapOutPerSecond.toFixed(1)}
            detail="kernel counter delta"
          />
          <Metric
            label="PSI"
            value={
              memory.psi.available
                ? `${memory.psi.someAvg10.toFixed(2)}%`
                : "N/A"
            }
            detail={
              memory.psi.available
                ? `some avg10 / full ${memory.psi.fullAvg10.toFixed(2)}%`
                : "kernel interface unavailable"
            }
          />
        </div>
      </section>
    </>
  );
}
function Paging() {
  const [a, setA] = useState("clock"),
    [f, setF] = useState(4),
    [r, setR] = useState("7,0,1,2,0,3,0,4"),
    [result, setResult] = useState<PagerResult | null>(null),
    [step, setStep] = useState(0);
  const framesInput = useRef<HTMLInputElement>(null);
  const run = async () => setResult(await core.pager(a, f, r));
  const current = result?.steps[step];
  return (
    <section className="panel paging-lab">
      <div className="panel-head">
        <div>
          <p className="eyebrow">EDUCATIONAL SIMULATION</p>
          <h2>Paging Lab</h2>
        </div>
      </div>
      <div className="lab-controls">
        <label>
          Algorithm
          <select value={a} onChange={(e) => setA(e.target.value)}>
            <option value="clock">Clock</option>
            <option value="fifo">FIFO</option>
            <option value="lru">LRU</option>
          </select>
        </label>
        <label>
          Frames
          <input
            ref={framesInput}
            type="number"
            value={f}
            min="1"
            max="128"
            onFocus={() => framesInput.current?.select()}
            onChange={(e) => setF(Number(e.target.value))}
          />
        </label>
        <label className="reference-input">
          References
          <input value={r} onChange={(e) => setR(e.target.value)} />
        </label>
        <button className="run-button" onClick={run}>
          Run
        </button>
      </div>
      {result && (
        <>
          <div className="lab-stats">
            <Metric label="Hits" value={result.hits} detail="simulated" />
            <Metric label="Faults" value={result.faults} detail="simulated" />
            <Metric
              label="Evictions"
              value={result.evictions}
              detail="simulated"
            />
            <Metric
              label="Write-backs"
              value={result.writeBacks}
              detail="simulated"
            />
          </div>
          <div className="step-bar">
            <b>
              {step + 1}/{result.steps.length}
            </b>
            <span>
              {current?.fault ? "PAGE FAULT" : "PAGE HIT"} page{" "}
              {current?.reference}
            </span>
            <Badge
              tone={current && typeof current.writeBack === "boolean" ? "blue" : "red"}
            >
              {current && typeof current.writeBack === "boolean"
                ? current.writeBack
                  ? "WRITE-BACK SUCCESS"
                  : "WRITE-BACK NOT REQUIRED"
                : "WRITE-BACK UNAVAILABLE"}
            </Badge>
            <button disabled={!step} onClick={() => setStep(step - 1)}>
              Previous
            </button>
            <button
              disabled={step >= result.steps.length - 1}
              onClick={() => setStep(step + 1)}
            >
              Next
            </button>
          </div>
          <div className="frame-table">
            {current?.frames.map((p, i) => (
              <div className="frame-row" key={i}>
                <span>Frame {i}</span>
                <b>{p < 0 ? "empty" : `Page ${p}`}</b>
                <Badge tone={current.frame === i ? "blue" : "neutral"}>
                  {current.frame === i ? "SELECTED" : "-"}
                </Badge>
              </div>
            ))}
          </div>
        </>
      )}
    </section>
  );
}
function Policies() {
  const [config, setConfig] = useState<Record<string, string>>({});
  const [message, setMessage] = useState("");
  useEffect(() => {
    core
      .config()
      .then((text) => {
        const next: Record<string, string> = {};
        text.split("\n").forEach((line) => {
          const [i, v] = line.split("=");
          if (i && v) next[i] = v;
        });
        setConfig(next);
      })
      .catch((e) => setMessage(String(e)));
  }, []);
  const save = async () => {
    try {
      await core.updateConfig(
        Object.entries(config).map(([k, v]) => `${k}=${v}`),
      );
      setMessage("Configuration saved by the C core");
    } catch (e) {
      setMessage(String(e));
    }
  };
  const fields = [
    "sample_interval",
    "high_threshold",
    "critical_threshold",
    "high_samples_required",
    "nice_step",
    "max_nice",
    "cooldown_seconds",
    "allow_auto_pause",
    "memory_high_available_percent",
    "memory_critical_available_percent",
    "memory_samples_required",
  ];
  return (
    <section className="panel policies">
      <p className="eyebrow">CORE CONFIGURATION</p>
      <h2>Policy controls</h2>
      <div className="policy-form">
        {fields.map((key) => (
          <label key={key}>
            {key}
            <input
              value={config[key] || ""}
              onChange={(e) => setConfig({ ...config, [key]: e.target.value })}
            />
          </label>
        ))}
      </div>
      <button className="run-button" disabled={!core.isDesktopShell} onClick={save}>
        Save through C core
      </button>
      {message && <p className="safety-note">{message}</p>}
      <div className="callout">
        Process actions, validation, and safety policy remain in the C core.
      </div>
    </section>
  );
}
function Settings({ connected }: { connected: boolean }) {
  const available = core.isDesktopShell && connected;
  return (
    <section className="panel settings">
      <p className="eyebrow">APPLICATION</p>
      <h2>Settings</h2>
      <div className="setting">
        <div>
          <b>Core connection</b>
           <span>
             {core.isDesktopShell
               ? "Restricted semantic Tauri bridge"
               : "Browser preview; live OS data requires Tauri"}
           </span>
         </div>
         <Badge tone={available ? "green" : core.isDesktopShell ? "red" : "blue"}>
           {available ? "CONNECTED" : core.isDesktopShell ? "DISCONNECTED" : "PREVIEW"}
        </Badge>
      </div>
    </section>
  );
}
createRoot(document.getElementById("root")!).render(<App />);
