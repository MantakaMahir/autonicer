use serde::{Deserialize, Serialize};
use std::{os::unix::fs::PermissionsExt, path::PathBuf, process::Command};
use tauri::{AppHandle, Manager};

#[derive(Serialize, Deserialize)]
pub struct SystemStatus {
    #[serde(rename = "cpuPercent")]
    pub cpu_percent: f64,
    #[serde(rename = "loadState")]
    pub load_state: String,
    pub timestamp: i64,
}

#[derive(Serialize, Deserialize)]
pub struct MemoryStatus {
    pub timestamp: i64,
    #[serde(rename = "memoryState")] pub memory_state: String,
    #[serde(rename = "totalKb")] pub total_kb: u64,
    #[serde(rename = "availableKb")] pub available_kb: u64,
    #[serde(rename = "availablePercent")] pub available_percent: f64,
    #[serde(rename = "swapTotalKb")] pub swap_total_kb: u64,
    #[serde(rename = "swapUsedKb")] pub swap_used_kb: u64,
    #[serde(rename = "minorFaultsPerSecond")] pub minor_faults_per_second: f64,
    #[serde(rename = "majorFaultsPerSecond")] pub major_faults_per_second: f64,
    #[serde(rename = "swapInPerSecond")] pub swap_in_per_second: f64,
    #[serde(rename = "swapOutPerSecond")] pub swap_out_per_second: f64,
}

fn core_path(app: &AppHandle) -> Result<PathBuf, String> {
    let development = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../autonicer");
    if development.is_file() { return Ok(development); }
    let resources = app.path().resource_dir().map_err(|e| e.to_string())?;
    if let Ok(entries) = std::fs::read_dir(&resources) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_file() && path.metadata().map(|m| m.permissions().mode() & 0o111 != 0).unwrap_or(false) && path.file_name().and_then(|n| n.to_str()).map(|n| n.starts_with("autonicer-")).unwrap_or(false) { return Ok(path); }
        }
    }
    let cwd = std::env::current_dir().map_err(|e| e.to_string())?.join("../autonicer");
    if cwd.exists() { return Ok(cwd); }
    Err("AutoNicer core executable was not found".into())
}

fn run_core(app: &AppHandle, args: &[&str]) -> Result<String, String> {
    let output = Command::new(core_path(app)?).args(args).output().map_err(|e| e.to_string())?;
    if !output.status.success() { return Err(String::from_utf8_lossy(&output.stderr).trim().to_string()); }
    Ok(String::from_utf8_lossy(&output.stdout).to_string())
}

#[tauri::command]
fn get_system_status(app: AppHandle) -> Result<SystemStatus, String> {
    serde_json::from_str(run_core(&app, &["sample"])?.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
fn get_processes(app: AppHandle) -> Result<serde_json::Value, String> {
    serde_json::from_str(run_core(&app, &["list", "--json"])?.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
fn get_memory_status(app: AppHandle) -> Result<MemoryStatus, String> {
    serde_json::from_str(run_core(&app, &["memory", "--json"])?.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
fn get_memory_candidates(app: AppHandle) -> Result<serde_json::Value, String> {
    serde_json::from_str(run_core(&app, &["memory-candidates", "--json"])?.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
fn run_pager_simulation(app: AppHandle, algorithm: String, frames: u32, reference: String) -> Result<serde_json::Value, String> {
    if !matches!(algorithm.as_str(), "fifo" | "lru" | "clock") || !(1..=128).contains(&frames) || reference.is_empty() { return Err("invalid paging simulation request".into()); }
    let frame_text = frames.to_string();
    let output = run_core(&app, &["pager-demo", "--algorithm", &algorithm, "--frames", &frame_text, "--reference", &reference])?;
    serde_json::from_str(output.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
fn get_history(app: AppHandle) -> Result<String, String> { run_core(&app, &["history"]) }

#[tauri::command]
fn get_config(app: AppHandle) -> Result<String, String> { run_core(&app, &["config"]) }

#[tauri::command]
fn process_command(app: AppHandle, command: String, pid: u32, classification: Option<String>) -> Result<String, String> {
    let pid_text = pid.to_string();
    let args: Vec<&str> = match command.as_str() {
        "classify" => vec!["classify", &pid_text, classification.as_deref().ok_or("classification is required")?],
        "protect" | "unprotect" | "restore" | "resume" => vec![command.as_str(), &pid_text],
        _ => return Err("unsupported process command".into()),
    };
    run_core(&app, &args)
}

#[tauri::command]
fn start_monitoring(app: AppHandle) -> Result<String, String> { run_core(&app, &["list", "--json"]) }

#[tauri::command]
fn stop_monitoring() -> Result<(), String> { Ok(()) }

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() { tauri::Builder::default().invoke_handler(tauri::generate_handler![get_system_status,get_processes,get_memory_status,get_memory_candidates,run_pager_simulation,get_history,get_config,process_command,start_monitoring,stop_monitoring]).run(tauri::generate_context!()).expect("error while running AutoNicer"); }
