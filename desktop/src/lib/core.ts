import { invoke } from "@tauri-apps/api/core";
import type {
  MemoryStatus,
  PagerStep,
  PagerResult,
  ProcessInfo,
  SystemStatus,
} from "../types";
const desktopAvailable = () =>
  typeof window !== "undefined" && "__TAURI_INTERNALS__" in window;
export const isDesktopShell = desktopAvailable();
const previewNow = () => Math.floor(Date.now() / 1000);
const previewProcesses: ProcessInfo[] = [
  {
    pid: 4201,
    name: "cpu_hog",
    cpuPercent: 18.4,
    nice: 0,
    state: "R",
    rssKb: 18432,
    swapKb: 0,
    minorFaultsPerSecond: 12.4,
    majorFaultsPerSecond: 0,
    classification: "BACKGROUND",
    protected: false,
    priorityChanged: false,
    paused: false,
  },
  {
    pid: 4202,
    name: "memory_hog",
    cpuPercent: 2.1,
    nice: 0,
    state: "S",
    rssKb: 262144,
    swapKb: 0,
    minorFaultsPerSecond: 48.2,
    majorFaultsPerSecond: 0,
    classification: "NORMAL",
    protected: false,
    priorityChanged: false,
    paused: false,
  },
];
const previewPager = (
  algorithm: string,
  frameCount: number,
  reference: string,
): PagerResult => {
  const refs = reference
    .split(",")
    .map((value) => Number.parseInt(value.replace(/W$/, ""), 10))
    .filter(Number.isFinite);
  const frames = Array(frameCount).fill(-1) as number[];
  const steps: PagerStep[] = [];
  let pointer = 0;
  let hits = 0;
  let faults = 0;
  let evictions = 0;
  for (const page of refs) {
    const frame = frames.indexOf(page);
    if (frame >= 0) {
      hits++;
      steps.push({
        reference: page,
        write: false,
        hit: true,
        fault: false,
        frame,
        victim: -1,
        writeBack: false,
        frames: [...frames],
      });
      continue;
    }
    faults++;
    const target = frames.indexOf(-1) >= 0 ? frames.indexOf(-1) : pointer;
    const victim = frames[target];
    if (victim >= 0) evictions++;
    frames[target] = page;
    pointer = (target + 1) % frameCount;
    steps.push({
      reference: page,
      write: false,
      hit: false,
      fault: true,
      frame: target,
      victim,
      writeBack: false,
      frames: [...frames],
    });
  }
  return {
    algorithm,
    frames: frameCount,
    steps,
    hits,
    faults,
    evictions,
    writeBacks: 0,
  };
};
const previewValue = <T,>(command: string): T => {
  const now = previewNow();
  const values: Record<string, unknown> = {
    get_system_status: { cpuPercent: 21.7, loadState: "NORMAL", timestamp: now },
    get_memory_status: {
      timestamp: now,
      memoryState: "NORMAL",
      totalKb: 16384000,
      availableKb: 9216000,
      availablePercent: 56.2,
      swapTotalKb: 2097152,
      swapUsedKb: 0,
      minorFaultsPerSecond: 148.4,
      majorFaultsPerSecond: 0,
      swapInPerSecond: 0,
      swapOutPerSecond: 0,
    },
    get_processes: previewProcesses,
    get_history: "Browser preview: no live controller actions recorded.",
    get_config:
      "sample_interval=2\nhigh_threshold=70\ncritical_threshold=90\nhigh_samples_required=3\nnice_step=2\nmax_nice=19\ncooldown_seconds=30\nallow_auto_pause=0\nmemory_high_available_percent=20\nmemory_critical_available_percent=10\nmemory_samples_required=3\n",
    monitor_status: false,
  };
  return values[command] as T;
};
const call = <T>(command: string, args?: Record<string, unknown>) =>
  desktopAvailable()
    ? invoke<T>(command, args)
    : Promise.resolve(previewValue<T>(command));
export const core = {
  isDesktopShell,
  status: () => call<SystemStatus>("get_system_status"),
  processes: () => call<ProcessInfo[]>("get_processes"),
  memory: () => call<MemoryStatus>("get_memory_status"),
  memoryCandidates: () => call<unknown[]>("get_memory_candidates"),
  pager: (algorithm: string, frames: number, reference: string) =>
    desktopAvailable()
      ? call<PagerResult>("run_pager_simulation", { algorithm, frames, reference })
      : Promise.resolve(previewPager(algorithm, frames, reference)),
  history: () => call<string>("get_history"),
  config: () => call<string>("get_config"),
  command: (command: string, pid: number, classification?: string) =>
    desktopAvailable()
      ? call<string>("process_command", { command, pid, classification })
      : Promise.reject(new Error("Process controls require the Tauri desktop app.")),
  updateConfig: (values: string[]) =>
    desktopAvailable()
      ? call<string>("update_config", { values })
      : Promise.reject(new Error("Configuration changes require the Tauri desktop app.")),
  startMonitoring: (dryRun: boolean) =>
    desktopAvailable()
      ? call<string>("start_monitoring", { dryRun })
      : Promise.reject(new Error("Monitoring requires the Tauri desktop app.")),
  stopMonitoring: () => call<void>("stop_monitoring"),
  monitorStatus: () => call<boolean>("monitor_status"),
};
