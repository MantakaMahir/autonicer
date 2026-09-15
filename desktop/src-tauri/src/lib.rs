use serde::{Deserialize, Serialize};
use std::{
    os::unix::fs::PermissionsExt,
    path::PathBuf,
    process::{Child, Command, Stdio},
    sync::Mutex,
    thread,
};
use tauri::{AppHandle, Manager};

struct MonitorProcess(Mutex<Option<Child>>);
impl Drop for MonitorProcess {
    fn drop(&mut self) {
        if let Ok(mut slot) = self.0.lock() {
            if let Some(mut child) = slot.take() {
                let _ = child.kill();
                let _ = child.wait();
            }
        }
    }
}

#[derive(Serialize, Deserialize)]
pub struct SystemStatus {
    #[serde(rename = "cpuPercent")]
    pub cpu_percent: f64,
    #[serde(rename = "loadState")]
    pub load_state: String,
    pub timestamp: i64,
}

#[derive(Serialize, Deserialize)]
pub struct PsiStatus {
    pub available: bool,
    #[serde(rename = "someAvg10")]
    pub some_avg10: f64,
    #[serde(rename = "fullAvg10")]
    pub full_avg10: f64,
}

#[derive(Serialize, Deserialize)]
pub struct MemoryStatus {
    pub timestamp: i64,
    #[serde(rename = "memoryState")]
    pub memory_state: String,
    #[serde(rename = "totalKb")]
    pub total_kb: u64,
    #[serde(rename = "availableKb")]
    pub available_kb: u64,
    #[serde(rename = "availablePercent")]
    pub available_percent: f64,
    #[serde(rename = "swapTotalKb")]
    pub swap_total_kb: u64,
    #[serde(rename = "swapUsedKb")]
    pub swap_used_kb: u64,
    #[serde(rename = "minorFaultsPerSecond")]
    pub minor_faults_per_second: f64,
    #[serde(rename = "majorFaultsPerSecond")]
    pub major_faults_per_second: f64,
    #[serde(rename = "swapInPerSecond")]
    pub swap_in_per_second: f64,
    #[serde(rename = "swapOutPerSecond")]
    pub swap_out_per_second: f64,
    pub psi: PsiStatus,
}

fn core_path(app: &AppHandle) -> Result<PathBuf, String> {
    let development = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../autonicer");
    if development.is_file() {
        return Ok(development);
    }
    let resources = app.path().resource_dir().map_err(|e| e.to_string())?;
    if let Ok(entries) = std::fs::read_dir(&resources) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_file()
                && path
                    .metadata()
                    .map(|m| m.permissions().mode() & 0o111 != 0)
                    .unwrap_or(false)
                && path
                    .file_name()
                    .and_then(|n| n.to_str())
                    .map(|n| n.starts_with("autonicer-"))
                    .unwrap_or(false)
            {
                return Ok(path);
            }
        }
    }
    let cwd = std::env::current_dir()
        .map_err(|e| e.to_string())?
        .join("../autonicer");
    if cwd.exists() {
        return Ok(cwd);
    }
    Err("AutoNicer core executable was not found".into())
}

fn run_core(app: &AppHandle, args: &[&str]) -> Result<String, String> {
    let output = Command::new(core_path(app)?)
        .args(args)
        .output()
        .map_err(|e| e.to_string())?;
    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).trim().to_string());
    }
    Ok(String::from_utf8_lossy(&output.stdout).to_string())
}

async fn run_core_async(app: AppHandle, args: Vec<String>) -> Result<String, String> {
    tauri::async_runtime::spawn_blocking(move || {
        let refs: Vec<&str> = args.iter().map(String::as_str).collect();
        run_core(&app, &refs)
    })
    .await
    .map_err(|e| e.to_string())?
}

#[tauri::command]
async fn get_system_status(app: AppHandle) -> Result<SystemStatus, String> {
    let output = run_core_async(app, vec!["sample".into()]).await?;
    serde_json::from_str(output.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
async fn get_processes(app: AppHandle) -> Result<serde_json::Value, String> {
    let output = run_core_async(
        app,
        vec!["list".into(), "--all".into(), "--json".into()],
    )
    .await?;
    serde_json::from_str(output.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
async fn get_memory_status(app: AppHandle) -> Result<MemoryStatus, String> {
    let output = run_core_async(app, vec!["memory".into(), "--json".into()]).await?;
    serde_json::from_str(output.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
async fn get_memory_candidates(app: AppHandle) -> Result<serde_json::Value, String> {
    let output = run_core_async(app, vec!["memory-candidates".into(), "--json".into()]).await?;
    serde_json::from_str(output.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
async fn run_pager_simulation(
    app: AppHandle,
    algorithm: String,
    frames: u32,
    reference: String,
) -> Result<serde_json::Value, String> {
    if !matches!(algorithm.as_str(), "fifo" | "lru" | "clock")
        || !(1..=128).contains(&frames)
        || reference.is_empty()
    {
        return Err("invalid paging simulation request".into());
    }
    let frame_text = frames.to_string();
    let output = run_core_async(
        app,
        vec![
            "pager-demo".into(),
            "--algorithm".into(),
            algorithm,
            "--frames".into(),
            frame_text,
            "--reference".into(),
            reference,
        ],
    )
    .await?;
    serde_json::from_str(output.trim()).map_err(|e| e.to_string())
}

#[tauri::command]
async fn get_history(app: AppHandle) -> Result<String, String> {
    run_core_async(app, vec!["history".into()]).await
}

#[tauri::command]
async fn get_config(app: AppHandle) -> Result<String, String> {
    run_core_async(app, vec!["config".into()]).await
}

#[tauri::command]
async fn process_command(
    app: AppHandle,
    command: String,
    pid: u32,
    classification: Option<String>,
) -> Result<String, String> {
    let pid_text = pid.to_string();
    let args: Vec<String> = match command.as_str() {
        "classify" => vec![
            "classify".into(),
            pid_text,
            classification.ok_or("classification is required")?,
        ],
        "protect" | "unprotect" | "restore" | "resume" | "kill" => {
            vec![command, pid_text]
        }
        _ => return Err("unsupported process command".into()),
    };
    run_core_async(app, args).await
}

#[tauri::command]
async fn update_config(app: AppHandle, values: Vec<String>) -> Result<String, String> {
    if values.is_empty()
        || values
            .iter()
            .any(|v| !v.contains('=') || v.starts_with('-'))
    {
        return Err("invalid configuration values".into());
    }
    let allowed = [
        "sample_interval=",
        "high_threshold=",
        "critical_threshold=",
        "high_samples_required=",
        "nice_step=",
        "max_nice=",
        "cooldown_seconds=",
        "allow_auto_pause=",
        "memory_high_available_percent=",
        "memory_critical_available_percent=",
        "memory_samples_required=",
    ];
    if values
        .iter()
        .any(|v| !allowed.iter().any(|prefix| v.starts_with(prefix)))
    {
        return Err("unsupported configuration key".into());
    }
    let mut args = vec!["set-config".to_string()];
    args.extend(values);
    run_core_async(app, args).await
}

fn launch_monitor(app: &AppHandle, monitor: &MonitorProcess) -> Result<(), String> {
    let mut slot = monitor
        .0
        .lock()
        .map_err(|_| "monitor state unavailable".to_string())?;
    if slot.as_ref().is_some() {
        return Ok(());
    }
    let mut child = Command::new(core_path(app)?)
        .arg("monitor")
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| e.to_string())?;
    if let Some(output) = child.stdout.take() {
        thread::spawn(move || {
            use std::io::BufRead;
            for line in std::io::BufReader::new(output).lines() {
                let _ = line;
            }
        });
    }
    if let Some(output) = child.stderr.take() {
        thread::spawn(move || {
            use std::io::BufRead;
            for line in std::io::BufReader::new(output).lines() {
                let _ = line;
            }
        });
    }
    *slot = Some(child);
    Ok(())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .manage(MonitorProcess(Mutex::new(None)))
        .setup(|app| {
            let monitor = app.state::<MonitorProcess>();
            launch_monitor(app.handle(), &monitor).map_err(std::io::Error::other)?;
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            get_system_status,
            get_processes,
            get_memory_status,
            get_memory_candidates,
            run_pager_simulation,
            get_history,
            get_config,
            process_command,
            update_config
        ])
        .run(tauri::generate_context!())
        .expect("error while running AutoNicer");
}
