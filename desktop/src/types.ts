export type LoadState = "NORMAL" | "HIGH" | "CRITICAL" | "COOLDOWN";
export type Classification = "NORMAL" | "BACKGROUND" | "CRITICAL";
export type ProcessInfo = {
  pid: number;
  name: string;
  cpuPercent: number;
  nice: number;
  state: string;
  rssKb: number;
  swapKb: number;
  minorFaultsPerSecond: number;
  majorFaultsPerSecond: number;
  classification: Classification;
  protected: boolean;
  priorityChanged: boolean;
  paused: boolean;
};
export type SystemStatus = {
  cpuPercent: number;
  loadState: LoadState;
  timestamp: number;
};
export type Action = {
  time: string;
  raw: string;
  kind: string;
  success: boolean;
};
export type MemoryStatus = {
  timestamp: number;
  memoryState: string;
  totalKb: number;
  availableKb: number;
  availablePercent: number;
  swapTotalKb: number;
  swapUsedKb: number;
  minorFaultsPerSecond: number;
  majorFaultsPerSecond: number;
  swapInPerSecond: number;
  swapOutPerSecond: number;
};
export type PagerStep = {
  reference: number;
  write: boolean;
  hit: boolean;
  fault: boolean;
  frame: number;
  victim: number;
  writeBack: boolean;
  frames: number[];
};
export type PagerResult = {
  algorithm: string;
  frames: number;
  steps: PagerStep[];
  hits: number;
  faults: number;
  evictions: number;
  writeBacks: number;
};
