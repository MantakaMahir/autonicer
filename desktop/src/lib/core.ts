import { invoke } from '@tauri-apps/api/core'
import type { ProcessInfo, SystemStatus } from '../types'
const desktopAvailable = () => typeof window !== 'undefined' && '__TAURI_INTERNALS__' in window
const call = <T>(command: string, args?: Record<string, unknown>) => desktopAvailable()
  ? invoke<T>(command, args)
  : Promise.reject(new Error('AutoNicer desktop shell is not running. Start it with: npm run tauri:dev'))
export const core = {
  status: () => call<SystemStatus>('get_system_status'),
  processes: () => call<ProcessInfo[]>('get_processes'),
  history: () => call<string>('get_history'),
  config: () => call<string>('get_config'),
  command: (command:string,pid:number,classification?:string) => call<string>('process_command',{command,pid,classification}),
}
