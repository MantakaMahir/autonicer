export type LoadState = 'NORMAL'|'HIGH'|'CRITICAL'|'COOLDOWN'
export type Classification = 'NORMAL'|'BACKGROUND'|'CRITICAL'
export type ProcessInfo = {pid:number;name:string;cpuPercent:number;nice:number;state:string;rssKb:number;swapKb:number;minorFaultsPerSecond:number;majorFaultsPerSecond:number;classification:Classification;protected:boolean;priorityChanged:boolean;paused:boolean}
export type SystemStatus = {cpu_percent:number;load_state:LoadState;timestamp:number}
export type Action = {time:string;raw:string;kind:string;success:boolean}
export type MemoryStatus = {timestamp:number;memory_state:string;total_kb:number;available_kb:number;available_percent:number;swap_total_kb:number;swap_used_kb:number;minor_faults_per_second:number;major_faults_per_second:number;swap_in_per_second:number;swap_out_per_second:number}
export type PagerStep = {reference:number;write:boolean;hit:boolean;fault:boolean;frame:number;victim:number;writeBack:boolean;frames:number[]}
export type PagerResult = {algorithm:string;frames:number;steps:PagerStep[];hits:number;faults:number;evictions:number;writeBacks:number}
