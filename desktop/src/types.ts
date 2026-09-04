export type LoadState = 'NORMAL'|'HIGH'|'CRITICAL'|'COOLDOWN'
export type Classification = 'NORMAL'|'BACKGROUND'|'CRITICAL'
export type ProcessInfo = {pid:number;name:string;cpuPercent:number;nice:number;state:string;classification:Classification;protected:boolean;priorityChanged:boolean;paused:boolean}
export type SystemStatus = {cpu_percent:number;load_state:LoadState;timestamp:number}
export type Action = {time:string;raw:string;kind:string;success:boolean}
