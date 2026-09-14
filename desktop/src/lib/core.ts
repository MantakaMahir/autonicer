import { invoke } from "@tauri-apps/api/core";
import type {
  MemoryStatus,
  PagerResult,
  ProcessInfo,
  SystemStatus,
} from "../types";

const desktopAvailable = () =>
  typeof window !== "undefined" && "__TAURI_INTERNALS__" in window;
export const isDesktopShell = desktopAvailable();

const call = <T>(command: string, args?: Record<string, unknown>) =>
  desktopAvailable()
    ? invoke<T>(command, args)
    : Promise.reject(
        new Error(
          "AutoNicer desktop shell is not running. Start it with: ./run-desktop.sh",
        ),
      );

export const core = {
  isDesktopShell,
  status: () => call<SystemStatus>("get_system_status"),
  processes: () => call<ProcessInfo[]>("get_processes"),
  memory: () => call<MemoryStatus>("get_memory_status"),
  memoryCandidates: () => call<unknown[]>("get_memory_candidates"),
  pager: (algorithm: string, frames: number, reference: string) =>
    call<PagerResult>("run_pager_simulation", { algorithm, frames, reference }),
  history: () => call<string>("get_history"),
  config: () => call<string>("get_config"),
  command: (command: string, pid: number, classification?: string) =>
    call<string>("process_command", { command, pid, classification }),
  updateConfig: (values: string[]) => call<string>("update_config", { values }),
};
